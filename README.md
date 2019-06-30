# RateLimiter
The scope is to design and develop a simple rate limiter (POC) that can limit the rate to 100 requests/minute/resource.

## Design Considerations:
  Option 3 out of the 4 following potential design choices were selected:
  1. **Request Logs**: Log individual requests with their time stamps and get an aggregate anytime you want a request count over a period of time. This approach will work almost perfectly but will require additional space to store all requests that happens 60 secs + regular purge interval.
  2. **Rate Windows**: Is to have smaller windows that could sum up to the actual window. Say 5 secs or 1 secs buckets where the requests are aggregated per resource and anytime a sum is needed the recent relevant buckets counts are aggregated and returned or better we can store the current sum for given interval in a separate variable and can return queries in O(1) time. The problem with this approach there still is a chance to miss count and potentially allow more requests around boundaries but the probability of this happening can be minimized by using a smaller bucket a smaller bucket.
  3. **Sliding window with aggregate data points**: In option 1 instead of storing all requests with time stamp we can still aggregate into buckets and create a virtual sliding window by scaling the previous bucket. That is while working on the current bucket we can include a scaled (down) version of the previous bucket till this bucket time limit is reached and we continue doing the same for the next one and so on. As the counts and scaling are simple math we can still service request in O(1) time. The disadvantage of this approach is that there are corner cases where we could potentially allow more requests than allocated. But Law of large numbers dictates if we repeat the same experiment lot of times we should arrive at our expected result.
  4. **Message Queues**: We can use message queues at various points of interaction, this is little complicated so wont consider this for the POC.
  
  I ended up selecting option 3 because it offered a good trade-off between accuracy and ease of implementation.
  
## Architecture Considerations:
  1. **Centralized Rate Limiter**:
    In this case Rate-Limiter runs on a separate instance to which webservers talk to to get request allowed/denied responses. The prefered mode of communication is http but we can implement a custom TCP based soulution that can remove HTTP overhead.
      **Pros**:
        * Deploying as an independent service, we can scale that service independently from the web-servers. 
        * Updating/modifying the limiter can happen independently of the web-servers e.g. we don't have to patch all the web-servers to handle bugs in Limiter code.
        * Resource foot-print of the limiter wont affect the web-servers
      **Cons**:
        * Major issue would be communication as each request to webserver would mean an additional request to the rate-limiter and would potentially be a network bottle-neck while scaling. 
        * Also will add relatively more latency to each request as this adds more network delays and all unreliablity that comes with network communication. Latency can be minimized by reducing the number of network hops between the 2 servers/service.
        * If the webservers are unable to reach rate-limiter, there wont be any rate-limiting.
  2. **Local Rate Lmiter Service with DB sync**:
    In this case RateLimter runs as a local service in each of the web server and are synced across through a central DB layer.
      **Pros**:
        * We can do local optimizations and reduce the amount of external traffic.
        * Latency is minimal (which depending on the scale could be a huge advantage) as local communication is always faster (and reliable) than network.
        * Even if Database sync fails we can still rely on in-memory per node rate-limiting temporarly. 
      **Cons**:
        * We need to monitor this service as part of monitoring the webserver and operational procedure has to be established as to what to do when the webservice is fine but the rate-limiter inside the web-server fails.
        * Patching and upgrading the rate-limiter will require same effort as patching/upgrading the application.
        * Resource foot-print of the rate-limiter will affect application servers so capacity have to planned accordingly.
  3. **Queues**:
    This is again a variation of option 4 of the design where individual requests to the ratelimiter service are queued and responded in order. This could lead to potential bottle necks and is relatively hard to implement, so considered out of scope for this POC.
  
## Design Details:
  As mentioned earlier option 3, **Sliding window with aggregate data points** design is used for this POC. 
  1. Rate limit window i.e. per minute or per 10 secs etc., have to mentioned while RateLimiter object is created and stays constant for that instance.
  2. Each resource can be given individual limits say one can have a limit of 100 rpm while other can have 60 rpm.
  3. For each resource we maintain 2 buckets of each limit-window size (say 60 secs). Current count is added to the current bucket and is rolled over after 60 secs. Whether 60 secs has passed or not is calculated by storing the current window timestamp.
  4. At any given point current rate is determined by the current count and a scaled version of the previous count. For example, we are in the 30th second of the current window, current count is 30 and previous count is 60, so the current rate is:
  ```
  current_count + prev_count * (window_size - current_window_size) / window_size
  ```
  which in this case happens to be:
  ```
  30 + 60 * (60 - 30) / 60 = 60
  ```
  5. Each time a new request comes to the web-server a query is made with the resourceId to the rate-limiter to determine whether the request is allowed or not, reate limiter responds yes/no based on the current rate. As part of responding this to this query rate-limiter does some book keeping. It checks whether the current bucket window has expired in which case it creates a new bucket and updates the old bucket to a scaled down version of the the current bucket as needed.
  6. Multiple rate-limiters running on different boxes (on the application node or as a separate node) syncs to each other through a **Redis** DB layer.
  7. The service throttles responses i.e. for any given request rate the service allows no more than the allowed limit. For example for a resource of limit 100 rpm and a request rate of 1000 rpm the service only allows ~100 requests to pass through. This is done by counting only the allowed requests.
  For example after an one hour run with a rate limit of 100 rpm and a rate of 10 rps we get the following stats:
  ```
  elapsed secs: 3600 total requests: 35869 allowed requests: 6002
  key: limitPerWindowSize val: 100
  key: prevCount val: 100
  key: currCount val: 2
  key: windowTimeStamp val: 1561764043
  ```
  8. **Optimization**: As we are doing point 7 we can reduce the number of DB queries in a distributed environment by making a query only when the in-memory limiter thinks the request has to be allowed, otherwise we don't have to sync because we are not going to change the count or allow the request. We have to sync when we change time-window as well. This optimization will potentially reduce the service to DB traffic significantly and will be in order of limit per window rather than total requests.
  
### TODO:
  * Add Architecture and design diagrams to README.
  * Multi-threading or event driven implementaion.
  * Local/network interface to the service.
  * Multi threaded and multi-process testing.

## Compile and Run:
  The solution needs a simple redis C library called [Hiredis](https://github.com/redis/hiredis), please download and install before compiling the source.
  ```
  make test_func
  ```
  should run some basic tests and check connections. Sample success output:
  ```
  #### Testing Data Store Handler ####
  PING: PONG
  Data store connection test: PASSED
  Data store addLimitData test: PASSED
  Data store getLimitData test: PASSED
  Data store incRequestCount test: PASSED
  Data store decRequestCount test: PASSED
  #### Testing RateLimiter ####
  Running a 10 sec load with load of 1 rps, window: 5 and allowed rate: 2/window
  rateLimit for css: 2
  elapsed secs: 1 total Requests: 1 allowed Requests: 1
  elapsed secs: 2 total Requests: 2 allowed Requests: 2
  elapsed secs: 3 total Requests: 3 allowed Requests: 2
  elapsed secs: 4 total Requests: 4 allowed Requests: 2
  elapsed secs: 5 total Requests: 5 allowed Requests: 2
  elapsed secs: 6 total Requests: 6 allowed Requests: 3
  elapsed secs: 7 total Requests: 7 allowed Requests: 3
  elapsed secs: 8 total Requests: 8 allowed Requests: 4
  elapsed secs: 9 total Requests: 9 allowed Requests: 4
  elapsed secs: 10 total Requests: 10 allowed Requests: 4
  ```
  
  ```
  make test_load
  ```
  should run a 5 mins test enforcing 100 requests/minute with a load of 10 requests/sec. Sample success output:
  ```
  Running a 5 mins load, with load of 10 rps, window size: 60 secs and rate limit of 100 per min
  Press any key to continue... 
  . . .
  rateLimit for css: 100
  elapsed secs: 1 total Requests: 1 allowed Requests: 1
  elapsed secs: 2 total Requests: 2 allowed Requests: 2
  elapsed secs: 2 total Requests: 3 allowed Requests: 3
  elapsed secs: 2 total Requests: 4 allowed Requests: 4
  elapsed secs: 2 total Requests: 5 allowed Requests: 5
  elapsed secs: 2 total Requests: 6 allowed Requests: 6
  elapsed secs: 2 total Requests: 7 allowed Requests: 7
  elapsed secs: 2 total Requests: 8 allowed Requests: 8
  elapsed secs: 2 total Requests: 9 allowed Requests: 9
  elapsed secs: 2 total Requests: 10 allowed Requests: 10
  . . .
  elapsed secs: 298 total Requests: 2970 allowed Requests: 497
  elapsed secs: 299 total Requests: 2971 allowed Requests: 498
  elapsed secs: 299 total Requests: 2972 allowed Requests: 499
  elapsed secs: 299 total Requests: 2973 allowed Requests: 499
  elapsed secs: 299 total Requests: 2974 allowed Requests: 499
  elapsed secs: 299 total Requests: 2975 allowed Requests: 499
  elapsed secs: 299 total Requests: 2976 allowed Requests: 499
  elapsed secs: 299 total Requests: 2977 allowed Requests: 499
  elapsed secs: 299 total Requests: 2978 allowed Requests: 499
  elapsed secs: 299 total Requests: 2979 allowed Requests: 499
  elapsed secs: 299 total Requests: 2980 allowed Requests: 499
  elapsed secs: 300 total Requests: 2981 allowed Requests: 500
  elapsed secs: 300 total Requests: 2982 allowed Requests: 500
  elapsed secs: 300 total Requests: 2983 allowed Requests: 500
  elapsed secs: 300 total Requests: 2984 allowed Requests: 500
  elapsed secs: 300 total Requests: 2985 allowed Requests: 500
  elapsed secs: 300 total Requests: 2986 allowed Requests: 500
  elapsed secs: 300 total Requests: 2987 allowed Requests: 500
  elapsed secs: 300 total Requests: 2988 allowed Requests: 500
  elapsed secs: 300 total Requests: 2989 allowed Requests: 500
  elapsed secs: 300 total Requests: 2990 allowed Requests: 500
  ```

## Future Improvements:
  * Use event driven solution to handle different request and DB sync-up asynchronously.
  * Adopt better communication techniques for communication between the application server and the rate-limiter like [zero MQ](http://zeromq.org/) or shared memory.
  * Better DB sync techniques via message queue or Redis queue.
    
