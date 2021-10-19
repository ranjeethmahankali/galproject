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
psum = pgf.listSum(series)

results = pgf.read(series)
resultSum = pgf.read(psum)

print("The length of the series is %s" % len(results))
print("The series: %s"  % results)
# assert(resultSum == sum(results))
print("Sum: ", resultSum)
