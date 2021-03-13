import numpy as np
icoVerts = [-0.525731112119133606, 0.0, 0.850650808352039932, 0.525731112119133606, 0.0, 0.850650808352039932, -0.525731112119133606, 0.0, -0.850650808352039932, 0.525731112119133606, 0.0, -0.850650808352039932, 0.0, 0.850650808352039932, 0.525731112119133606, 0.0, 0.850650808352039932, -0.525731112119133606,
            0.0, -0.850650808352039932, 0.525731112119133606, 0.0, -0.850650808352039932, -0.525731112119133606, 0.850650808352039932, 0.525731112119133606, 0.0, -0.850650808352039932, 0.525731112119133606, 0.0, 0.850650808352039932, -0.525731112119133606, 0.0, -0.850650808352039932, -0.525731112119133606, 0.0]

icoTriangles = [0, 4, 1, 0, 9, 4, 9, 5, 4, 4, 5, 8, 4, 8, 1, 8, 10, 1, 8, 3, 10, 5, 3, 8, 5, 2, 3, 2, 7,
                3, 7, 10, 3, 7, 6, 10, 7, 11, 6, 11, 0, 6, 0, 1, 6, 6, 1, 10, 9, 0, 11, 9, 11, 2, 9, 2, 5, 7, 2, 11]


gfaces = [icoTriangles[i: i + 3] for i in range(0, len(icoTriangles), 3)]
gverts = [np.array(icoVerts[i: i + 3]) for i in range(0, len(icoVerts), 3)]


class Ico:
    def __init__(self, faces, verts):
        self.faces = faces
        self.verts = verts

    def subdivide(self):
        edgeMidPts = dict()
        verts = [v for v in self.verts]
        faces = []
        for f in self.faces:
            mids = []
            for i in range(3):
                v1 = f[i]
                v2 = f[(i + 1) % 3]
                edge = (min(v1, v2), max(v1, v2))
                if edge in edgeMidPts:
                    mids.append(edgeMidPts[edge])
                else:
                    mpt = 0.5 * (self.verts[v1] + self.verts[v2])
                    mpt /= np.linalg.norm(mpt)
                    edgeMidPts[edge] = len(verts)
                    mids.append(len(verts))
                    verts.append(mpt)

            faces.append([f[0], mids[0], mids[2]])
            faces.append([f[1], mids[1], mids[0]])
            faces.append([f[2], mids[2], mids[1]])
            faces.append(mids)

        return Ico(faces, verts)

    def code(self):
        # print(self.verts)
        print("static constexpr std::array<glm::vec3, %s> sVertices = {{\n%s\n}};" % (
            len(self.verts),
            ",\n".join([("{%sf, %sf, %sf}" % (v[0], v[1], v[2]))
                        for v in self.verts])
        ))

        print('')

        print("static constexpr std::array<uint32_t, %s> sIndices = {{\n%s\n}};" % (
            len(self.faces) * 3,
            ",\n".join([("%s, %s, %s" % (f[0], f[1], f[2]))
                        for f in self.faces])
        ))


ico = Ico(gfaces, gverts)
ico = ico.subdivide()
ico = ico.subdivide()
ico = ico.subdivide()
ico = ico.subdivide()
ico.code()
