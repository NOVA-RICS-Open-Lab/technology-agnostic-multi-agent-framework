from tamaf import LaunchEMA
import time

ema = LaunchEMA(debug=0)

while ema.IsAlive():
    time.sleep(1)