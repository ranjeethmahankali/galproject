using Rhino.Geometry;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;

namespace RhinoInterface
{
    public static class MeshUtil
    {
        internal static Mesh ToRhinoMesh(this IntPtr meshptr)
        {
            IntPtr vertexPtr = IntPtr.Zero;
            IntPtr facePtr = IntPtr.Zero;
            int nVertices = 0;
            int nFaces = 0;
            Unsafe.Mesh_GetData(meshptr, ref vertexPtr, ref nVertices, ref facePtr, ref nFaces);

            double[] coords = new double[nVertices * 3];
            Marshal.Copy(vertexPtr, coords, 0, coords.Length);
            Unsafe.ReleaseDouble(vertexPtr, true);

            int[] faceIndices = new int[nFaces * 3];
            Marshal.Copy(facePtr, faceIndices, 0, faceIndices.Length);
            Unsafe.ReleaseInt(facePtr, true);

            int i = 0;
            List<Point3d> vertices = new List<Point3d>();
            while (i < coords.Length)
            {
                vertices.Add(new Point3d(coords[i++], coords[i++], coords[i++]));
            }

            i = 0;
            List<MeshFace> faces = new List<MeshFace>();
            while (i < faceIndices.Length)
            {
                faces.Add(new MeshFace(faceIndices[i++], faceIndices[i++], faceIndices[i++]));
            }

            Mesh mesh = new Mesh();
            mesh.Faces.AddFaces(faces);
            mesh.Vertices.AddVertices(vertices);
            mesh.Normals.ComputeNormals();
            mesh.FaceNormals.ComputeFaceNormals();
            return mesh;
        }

        internal static IntPtr ToUnmanagedMesh(this Mesh mesh)
        {
            double[] coords = mesh.Vertices.SelectMany(v => new double[] { v.X, v.Y, v.Z }).ToArray();
            mesh.Faces.ConvertQuadsToTriangles();
            int[] faceIndices = mesh.Faces.SelectMany(f => new int[] { f.A, f.B, f.C }).ToArray();
            return Unsafe.Mesh_Create(coords, mesh.Vertices.Count, faceIndices, mesh.Faces.Count);
        }

        public static Mesh CloneMesh(Mesh mesh)
        {
            IntPtr meshPtr = mesh.ToUnmanagedMesh();
            Mesh clone = meshPtr.ToRhinoMesh();
            Unsafe.Mesh_Delete(meshPtr);
            return clone;
        }

        public static double MeshVolume(Mesh mesh)
        {
            mesh.Vertices.CombineIdentical(true, true);
            IntPtr meshPtr = mesh.ToUnmanagedMesh();
            double volume = Unsafe.Mesh_Volume(meshPtr);
            Unsafe.Mesh_Delete(meshPtr);
            return volume;
        }
    }
}
