/*
Copyright (c) 2006, 2007 Alec Cove

Permission is hereby granted, free of charge, to any person obtaining a copy of this 
software and associated documentation files (the "Software"), to deal in the Software 
without restriction, including without limitation the rights to use, copy, modify, 
merge, publish, distribute, sublicense, and/or sell copies of the Software, and to 
permit persons to whom the Software is furnished to do so, subject to the following 
conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF 
CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*
TODO:
- fix the friction bug for two non-fixed particles in collision. The tangental
  component should not be scaled/applied in all instances, depending on the velocity
  of the other colliding item
*/

package org.cove.ape {
	
	// thanks to Jim Bonacci for changes using the inverse mass instead of mass
	
	internal final class CollisionResolver {
        
        internal static function resolve (pa:AbstractParticle, pb:AbstractParticle, 
                normal:APEVector, depth:Number):void {
     	
            var mtd:APEVector = normal.mult(depth);           
            var te:Number = pa.elasticity + pb.elasticity;
            var sumInvMass:Number = pa.invMass + pb.invMass;
            
            // the total friction in a collision is combined but clamped to [0,1]
            var tf:Number = MathUtil.clamp(1 - (pa.friction + pb.friction), 0, 1);
            
            // get the collision components, vn and vt
            var ca:Collision = pa.getComponents(normal);
            var cb:Collision = pb.getComponents(normal);

             // calculate the coefficient of restitution as the normal component
            var vnA:APEVector = (cb.vn.mult((te + 1) * pa.invMass).plus(
            		ca.vn.mult(pb.invMass - te * pa.invMass))).divEquals(sumInvMass);
            var vnB:APEVector = (ca.vn.mult((te + 1) * pb.invMass).plus(
            		cb.vn.mult(pa.invMass - te * pb.invMass))).divEquals(sumInvMass);
            
            // apply friction to the tangental component
            ca.vt.multEquals(tf);
            cb.vt.multEquals(tf);
            
            // scale the mtd by the ratio of the masses. heavier particles move less 
            var mtdA:APEVector = mtd.mult( pa.invMass / sumInvMass);     
            var mtdB:APEVector = mtd.mult(-pb.invMass / sumInvMass);
            
            // add the tangental component to the normal component for the new velocity 
            vnA.plusEquals(ca.vt);
            vnB.plusEquals(cb.vt);
           
            pa.resolveCollision(mtdA, vnA, normal, depth, -1, pb);
            pb.resolveCollision(mtdB, vnB, normal, depth,  1, pa);
        }
    }
}

