using System;
using System.Collections.Generic;
using System.Linq;
using System.Threading.Tasks;
using Rhino.Geometry;
using System.Runtime.InteropServices;
using System.Diagnostics;

namespace RhinoInterface
{
    public static class Test
    {
        public static Mesh CreateConvexHull(IEnumerable<Point3d> points, out long elapsedMs, out long elapsedTicks)
        {
            Point3d[] ptArr = points.ToArray();
            double[] coords = ptArr.SelectMany(pt => new double[] { pt.X, pt.Y, pt.Z }).ToArray();
            ulong nPoints = (ulong)ptArr.Length;
            IntPtr faceIndPtr = IntPtr.Zero;
            int nFaces = 0;

            var w = Stopwatch.StartNew();
            Unsafe.ConvexHull_Create(coords, nPoints, ref faceIndPtr, ref nFaces);
            int[] faceIndices = new int[nFaces * 3];
            Marshal.Copy(faceIndPtr, faceIndices, 0, faceIndices.Length);
            Unsafe.ReleaseInt(faceIndPtr, true);

            Mesh mesh = new Mesh();
            mesh.Vertices.AddVertices(ptArr);

            for (int fi = 0; fi < nFaces; fi++)
            {
                mesh.Faces.AddFace(faceIndices[3 * fi], faceIndices[3 * fi + 1], faceIndices[3 * fi + 2]);
            }
            w.Stop();
            elapsedMs = w.ElapsedMilliseconds;
            elapsedTicks = w.ElapsedTicks;

            mesh.RebuildNormals();

            return mesh;
        }
    }
}
