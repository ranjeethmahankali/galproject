from galutils import *

# fmt: off
# AppendBinPath("Debug")
AppendBinPath()
import pygalfunc as pgf
# fmt: on

start = pgf.var(10)
step = pgf.var(3)
count = pgf.var(10)

series = pgf.series(start, step, count)
psum = pgf.listSum(series)
combs = pgf.combinations(series, pgf.var(3))

results = pgf.read(series)
pycombs = pgf.read(combs)

print("The length of the series is %s" % len(results))
print("The series: %s"  % results)
assert(pgf.read(psum) == sum(results))
assert(120 == len(pycombs))
# print("Sum: ", resultSum)
print("Combinations of 3: ", pycombs)
