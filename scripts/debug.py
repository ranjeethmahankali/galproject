import pygalfunc as pgf

cloud = pgf.var_vec3([(0.3401877284049988, -0.3024486303329468, 0.0),
                      (-0.10561707615852356, -0.1647772490978241, 0.0),
                      (0.2830992341041565, 0.268229603767395, 0.0),
                      (0.29844003915786743, -0.22222527861595154, 0.0),
                      (0.41164737939834595, 0.05396997928619385, 0.0)])
vertices = pgf.var_vec3(
    [[(-0.5, -0.5, 0.0)], [(-0.4000000059604645, -0.5, 0.0)],
     [(-0.30000001192092896, -0.5, 0.0)], [(-0.19999998807907104, -0.5, 0.0)],
     [(-0.09999999403953552, -0.5, 0.0)], [(0.0, -0.5, 0.0)],
     [(0.10000002384185791, -0.5, 0.0)], [(0.19999998807907104, -0.5, 0.0)],
     [(0.30000001192092896, -0.5, 0.0)], [(0.40000003576278687, -0.5, 0.0)],
     [(0.5, -0.5, 0.0)], [(-0.5, -0.4000000059604645, 0.0)],
     [(-0.4000000059604645, -0.4000000059604645, 0.0)],
     [(-0.30000001192092896, -0.4000000059604645, 0.0)],
     [(-0.19999998807907104, -0.4000000059604645, 0.0)],
     [(-0.09999999403953552, -0.4000000059604645, 0.0)],
     [(0.0, -0.4000000059604645, 0.0)],
     [(0.10000002384185791, -0.4000000059604645, 0.0)],
     [(0.19999998807907104, -0.4000000059604645, 0.0)],
     [(0.30000001192092896, -0.4000000059604645, 0.0)],
     [(0.40000003576278687, -0.4000000059604645, 0.0)],
     [(0.5, -0.4000000059604645, 0.0)], [(-0.5, -0.30000001192092896, 0.0)],
     [(-0.4000000059604645, -0.30000001192092896, 0.0)],
     [(-0.30000001192092896, -0.30000001192092896, 0.0)],
     [(-0.19999998807907104, -0.30000001192092896, 0.0)],
     [(-0.09999999403953552, -0.30000001192092896, 0.0)],
     [(0.0, -0.30000001192092896, 0.0)],
     [(0.10000002384185791, -0.30000001192092896, 0.0)],
     [(0.19999998807907104, -0.30000001192092896, 0.0)],
     [(0.30000001192092896, -0.30000001192092896, 0.0)],
     [(0.40000003576278687, -0.30000001192092896, 0.0)],
     [(0.5, -0.30000001192092896, 0.0)], [(-0.5, -0.19999998807907104, 0.0)],
     [(-0.4000000059604645, -0.19999998807907104, 0.0)],
     [(-0.30000001192092896, -0.19999998807907104, 0.0)],
     [(-0.19999998807907104, -0.19999998807907104, 0.0)],
     [(-0.09999999403953552, -0.19999998807907104, 0.0)],
     [(0.0, -0.19999998807907104, 0.0)],
     [(0.10000002384185791, -0.19999998807907104, 0.0)],
     [(0.19999998807907104, -0.19999998807907104, 0.0)],
     [(0.30000001192092896, -0.19999998807907104, 0.0)],
     [(0.40000003576278687, -0.19999998807907104, 0.0)],
     [(0.5, -0.19999998807907104, 0.0)], [(-0.5, -0.09999999403953552, 0.0)],
     [(-0.4000000059604645, -0.09999999403953552, 0.0)],
     [(-0.30000001192092896, -0.09999999403953552, 0.0)],
     [(-0.19999998807907104, -0.09999999403953552, 0.0)],
     [(-0.09999999403953552, -0.09999999403953552, 0.0)],
     [(0.0, -0.09999999403953552, 0.0)],
     [(0.10000002384185791, -0.09999999403953552, 0.0)],
     [(0.19999998807907104, -0.09999999403953552, 0.0)],
     [(0.30000001192092896, -0.09999999403953552, 0.0)],
     [(0.40000003576278687, -0.09999999403953552, 0.0)],
     [(0.5, -0.09999999403953552, 0.0)], [(-0.5, 0.0, 0.0)],
     [(-0.4000000059604645, 0.0, 0.0)], [(-0.30000001192092896, 0.0, 0.0)],
     [(-0.19999998807907104, 0.0, 0.0)], [(-0.09999999403953552, 0.0, 0.0)],
     [(0.0, 0.0, 0.0)], [(0.10000002384185791, 0.0, 0.0)],
     [(0.19999998807907104, 0.0, 0.0)], [(0.30000001192092896, 0.0, 0.0)],
     [(0.40000003576278687, 0.0, 0.0)], [(0.5, 0.0, 0.0)],
     [(-0.5, 0.10000002384185791, 0.0)],
     [(-0.4000000059604645, 0.10000002384185791, 0.0)],
     [(-0.30000001192092896, 0.10000002384185791, 0.0)],
     [(-0.19999998807907104, 0.10000002384185791, 0.0)],
     [(-0.09999999403953552, 0.10000002384185791, 0.0)],
     [(0.0, 0.10000002384185791, 0.0)],
     [(0.10000002384185791, 0.10000002384185791, 0.0)],
     [(0.19999998807907104, 0.10000002384185791, 0.0)],
     [(0.30000001192092896, 0.10000002384185791, 0.0)],
     [(0.40000003576278687, 0.10000002384185791, 0.0)],
     [(0.5, 0.10000002384185791, 0.0)], [(-0.5, 0.19999998807907104, 0.0)],
     [(-0.4000000059604645, 0.19999998807907104, 0.0)],
     [(-0.30000001192092896, 0.19999998807907104, 0.0)],
     [(-0.19999998807907104, 0.19999998807907104, 0.0)],
     [(-0.09999999403953552, 0.19999998807907104, 0.0)],
     [(0.0, 0.19999998807907104, 0.0)],
     [(0.10000002384185791, 0.19999998807907104, 0.0)],
     [(0.19999998807907104, 0.19999998807907104, 0.0)],
     [(0.30000001192092896, 0.19999998807907104, 0.0)],
     [(0.40000003576278687, 0.19999998807907104, 0.0)],
     [(0.5, 0.19999998807907104, 0.0)], [(-0.5, 0.30000001192092896, 0.0)],
     [(-0.4000000059604645, 0.30000001192092896, 0.0)],
     [(-0.30000001192092896, 0.30000001192092896, 0.0)],
     [(-0.19999998807907104, 0.30000001192092896, 0.0)],
     [(-0.09999999403953552, 0.30000001192092896, 0.0)],
     [(0.0, 0.30000001192092896, 0.0)],
     [(0.10000002384185791, 0.30000001192092896, 0.0)],
     [(0.19999998807907104, 0.30000001192092896, 0.0)],
     [(0.30000001192092896, 0.30000001192092896, 0.0)],
     [(0.40000003576278687, 0.30000001192092896, 0.0)],
     [(0.5, 0.30000001192092896, 0.0)], [(-0.5, 0.40000003576278687, 0.0)],
     [(-0.4000000059604645, 0.40000003576278687, 0.0)],
     [(-0.30000001192092896, 0.40000003576278687, 0.0)],
     [(-0.19999998807907104, 0.40000003576278687, 0.0)],
     [(-0.09999999403953552, 0.40000003576278687, 0.0)],
     [(0.0, 0.40000003576278687, 0.0)],
     [(0.10000002384185791, 0.40000003576278687, 0.0)],
     [(0.19999998807907104, 0.40000003576278687, 0.0)],
     [(0.30000001192092896, 0.40000003576278687, 0.0)],
     [(0.40000003576278687, 0.40000003576278687, 0.0)],
     [(0.5, 0.40000003576278687, 0.0)], [(-0.5, 0.5, 0.0)],
     [(-0.4000000059604645, 0.5, 0.0)], [(-0.30000001192092896, 0.5, 0.0)],
     [(-0.19999998807907104, 0.5, 0.0)], [(-0.09999999403953552, 0.5, 0.0)],
     [(0.0, 0.5, 0.0)], [(0.10000002384185791, 0.5, 0.0)],
     [(0.19999998807907104, 0.5, 0.0)], [(0.30000001192092896, 0.5, 0.0)],
     [(0.40000003576278687, 0.5, 0.0)], [(0.5, 0.5, 0.0)]])

if __name__ == "__main__":
    distances = pgf.distance(pgf.graft(vertices), cloud)
    print(pgf.read(distances))