using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Runtime.InteropServices;

namespace RhinoInterface
{
    internal static class Unsafe
    {
        private const string dllName = "geom_algo_lib.dll";

        [DllImport(dllName, CallingConvention = CallingConvention.Cdecl)]
        internal static extern int Test_GetSquare(int n);
    }
}
