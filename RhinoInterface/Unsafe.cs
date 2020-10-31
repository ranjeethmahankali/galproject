using System;
using System.Runtime.InteropServices;
using System.Text;

namespace RhinoInterface
{
    public enum MeshCentroidType
    {
        VertexBased = 0,
        AreaBased,
        VolumeBased,
    };

    public enum MeshElementType
    {
        Vertex,
        Face,
    };

    internal static class Unsafe
    {
        private const string dllName = "geom_algo_lib.dll";

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int ConvexHull_Create(
            [MarshalAs(UnmanagedType.LPArray)] double[] coordinates,
            ulong numPoints,
            ref IntPtr faceIndices,
            ref int nFaces);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ReleaseInt(IntPtr arr, bool isArray);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void ReleaseDouble(IntPtr arr, bool isArray);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_GetData(
            IntPtr meshptr,
            ref IntPtr vertices,
            ref int nVertices,
            ref IntPtr faces,
            ref int nFaces);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr Mesh_Create(
            [MarshalAs(UnmanagedType.LPArray)] double[] vertCoords,
            int numVerts,
            [MarshalAs(UnmanagedType.LPArray)] int[] faceIndices,
            int numFaces);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_Delete(IntPtr meshPtr);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern double Mesh_Volume(IntPtr meshPtr);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_Centroid(IntPtr meshPtr, MeshCentroidType type, ref double x, ref double y, ref double z);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_QueryBox(
            IntPtr meshPtr,
            [MarshalAs(UnmanagedType.LPArray)] double[] bounds,
            ref IntPtr retIndices,
            ref int numIndices,
            MeshElementType element);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_QuerySphere(
            IntPtr meshPtr,
            double cx,
            double cy,
            double cz,
            double radius,
            ref IntPtr retIndices,
            ref int numIndices,
            MeshElementType element);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern bool Mesh_ContainsPoint(IntPtr meshPtr, double x, double y, double z);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern IntPtr Mesh_ClipWithPlane(
            IntPtr umesh,
            [MarshalAs(UnmanagedType.LPArray)] double[] pt,
            [MarshalAs(UnmanagedType.LPArray)] double[] norm);

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern void Mesh_ClosestPoint(
            IntPtr meshPtr,
            [MarshalAs(UnmanagedType.LPArray)] double[] point,
            [MarshalAs(UnmanagedType.LPArray)] double[] closePoint,
            double searchDistance);
    }
}
