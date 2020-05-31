using Rhino.Geometry;
using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;

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

        public static double GetVolume(Mesh mesh, out string summary)
        {
            mesh.Vertices.CombineIdentical(true, true);
            IntPtr meshPtr = mesh.ToUnmanagedMesh();
            Stopwatch w = Stopwatch.StartNew();
            double volume = Unsafe.Mesh_Volume(meshPtr);
            w.Stop();
            Unsafe.Mesh_Delete(meshPtr);
            summary = $"Volume computation: {w.ElapsedMilliseconds}ms | {w.ElapsedTicks} ticks.";
            return volume;
        }

        public static Point3d GetCentroid(Mesh mesh, MeshCentroidType type)
        {
            mesh.Vertices.CombineIdentical(true, true);
            IntPtr meshPtr = mesh.ToUnmanagedMesh();
            double x = 0, y = 0, z = 0;
            Unsafe.Mesh_Centroid(meshPtr, type, ref x, ref y, ref z);
            Unsafe.Mesh_Delete(meshPtr);
            return new Point3d(x, y, z);
        }

        private static int[] QueryMesh(Mesh mesh, BoundingBox box, MeshElementType element)
        {
            IntPtr meshPtr = mesh.ToUnmanagedMesh();
            IntPtr retIndicesPtr = IntPtr.Zero;
            int nIndices = 0;
            Unsafe.Mesh_QueryBox(meshPtr, box.ToArray(), ref retIndicesPtr, ref nIndices, element);

            int[] retIndices = new int[nIndices];
            Marshal.Copy(retIndicesPtr, retIndices, 0, nIndices);
            Unsafe.ReleaseInt(retIndicesPtr, true);
            Unsafe.Mesh_Delete(meshPtr);
            return retIndices;
        }

        private static int[] QueryMesh(Mesh mesh, Point3d center, double radius, MeshElementType element)
        {
            IntPtr meshptr = mesh.ToUnmanagedMesh();
            IntPtr retIndicesPtr = IntPtr.Zero;
            int nIndices = 0;
            Unsafe.Mesh_QuerySphere(meshptr, center.X, center.Y, center.Z, radius, ref retIndicesPtr, ref nIndices, element);
            
            int[] retIndices = new int[nIndices];
            Marshal.Copy(retIndicesPtr, retIndices, 0, nIndices);
            Unsafe.ReleaseInt(retIndicesPtr, true);
            Unsafe.Mesh_Delete(meshptr);
            return retIndices;
        }

        public static Mesh QueryMeshFaces(Mesh mesh, BoundingBox box)
        {
            int[] faceIndices = QueryMesh(mesh, box, MeshElementType.Face);
            return faceIndices.Any() ? mesh.Faces.ExtractFaces(faceIndices) : null;
        }

        public static Mesh QueryMeshFaces(Mesh mesh, Point3d center, double radius)
        {
            int[] faceIndices = QueryMesh(mesh, center, radius, MeshElementType.Face);
            return faceIndices.Any() ? mesh.Faces.ExtractFaces(faceIndices) : null;
        }

        public static Point3d[] QueryMeshVertices(Mesh mesh, BoundingBox box)
        {
            return QueryMesh(mesh, box, MeshElementType.Vertex).Select(i => (Point3d)mesh.Vertices[i]).ToArray();
        }

        public static Point3d[] QueryMeshVertices(Mesh mesh, Point3d center, double radius)
        {
            return QueryMesh(mesh, center, radius, MeshElementType.Vertex).Select(i => (Point3d)mesh.Vertices[i]).ToArray();
        }

        public static bool MeshContainsPoint(Mesh mesh, Point3d point)
        {
            IntPtr meshPtr = mesh.ToUnmanagedMesh();
            bool result = Unsafe.Mesh_ContainsPoint(meshPtr, point.X, point.Y, point.Z);
            Unsafe.Mesh_Delete(meshPtr);
            return result;
        }
    }
}
