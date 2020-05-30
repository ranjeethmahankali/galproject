using Rhino.Geometry;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace RhinoInterface
{
    public static class Extensions
    {
        public static double[] ToArray(this BoundingBox box)
        {
            return new double[]
            {
                box.Min.X,
                box.Min.Y,
                box.Min.Z,
                box.Max.X,
                box.Max.Y,
                box.Max.Z,
            };
        }
    }
}
