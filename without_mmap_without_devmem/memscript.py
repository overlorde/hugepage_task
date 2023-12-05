import psutil
import subprocess
import time

SLICE_IN_SECONDS = 1
p = subprocess.Popen('/home/dante/test/hptest')
resultTable = []
while p.poll() == None:
  resultTable.append(psutil.memory_info(p.pid))
  time.sleep(SLICE_IN_SECONDS)
