from tamaf import LaunchEMA, Agent
import time

ema = LaunchEMA(debug=True)

while ema.IsAlive():
    time.sleep(1)