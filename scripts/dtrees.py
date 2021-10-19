from galutils import *

# fmt: off
# AppendBinPath("Debug")
AppendBinPath()
import pygalfunc as pgf
# fmt: on

start = pgf.var(10)
step = pgf.var(2)
count = pgf.var(23)

series = pgf.series(start, step, count)

results = pgf.read(series)
print("The length of the series is %s" % len(results))
print("The series: %s"  % results)
