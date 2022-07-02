import pygalfunc as pgf

cloud = pgf.var_vec3([(0.3401877284049988, -0.3024486303329468, 0.0),
                      (-0.10561707615852356, -0.1647772490978241, 0.0),
                      (0.2830992341041565, 0.268229603767395, 0.0)])
vertices = pgf.var_vec3([[(-0.5, -0.5, 0.0)],
                         [(-0.4000000059604645, -0.5, 0.0)]])

if __name__ == "__main__":
    distances = pgf.distance(vertices, cloud)
    print(pgf.read(distances))
    # print(pgf.read(vertices))
