from tamaf import LaunchEMA
import time

ema = LaunchEMA(debug=1)

while ema.IsAlive():
    time.sleep(1)