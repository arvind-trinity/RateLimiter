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
    **Cons**:
  2. **Local Rate Lmiter Service with DB sync**:
    In this case RateLimter runs as a local service in each of the DB server and are synced across via a central DB layer.
    **Pros**:
    **Cons**:
  3. **Queues**:
    This is again a variation of option 4 of the design where individual requests to the ratelimiter service are queued and responded in order. This could lead to potential bottle necks and is relatively hard to implement, so considered out of scope for this POC.
  
## Design Details:

## Compile and run Tests:

## Future Improvements:
    
