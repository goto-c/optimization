# 00_random

<img src="thumbnail.gif">

This project simulates the simplest optimization process.  
  
## Initial configuration  
250 points are randomly generated on the OpenGL window  
  
## Simulation  
1. A random dot is generated in the frame.  
2. The square norm is calculated from each point to the dot.  
3. If the sum of the square norms is the minimum, the value is noted.  
4. Continue the same process (1 ~ 3) until there is no update within 1000 frames.  
