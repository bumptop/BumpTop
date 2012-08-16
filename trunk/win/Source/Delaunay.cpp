// Copyright 2012 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "BT_Common.h"
#include "Delaunay.h"



class Line2D
{
public:

	Vec2 p1, p2;
	Line2D(Vec2 p1, Vec2 p2)
	{
		this->p1 = p1;
		this->p2 = p2;
	}

	Line2D(NxReal x1, NxReal y1, NxReal x2, NxReal y2)
	{
		p1.x = x1; p1.y = y1;
		p2.x = x2; p2.y = y2;
	}

	void setLine(NxReal x1, NxReal y1, NxReal x2, NxReal y2)
	{
		p1.x = x1; p1.y = y1;
		p2.x = x2; p2.y = y2;
	}	

	inline NxReal getX2() const
	{
		return p2.x;
	}

	inline NxReal getX1() const
	{
		return p1.x;
	}

	inline NxReal getY2() const
	{
		return p2.y;
	}

	inline NxReal getY1() const
	{
		return p1.y;
	}

	inline Vec2 getP1() const
	{
		return p1;
	}

	inline Vec2 getP2() const
	{
		return p2;
	}

};

Line2D linePoint(NxReal width, NxReal height, double x, double y, double dx, double dy)
{
	NxReal outfl = 10.0f;
	NxReal ymin = -outfl * height;
	NxReal ymax = (outfl + 1.0f) * height;

	if (dx == 0.0f) {
		return Line2D(x, ymin, x, ymax);
	}

	NxReal slope = dy / dx;
	NxReal x1 = -outfl * width;
	NxReal y1 = slope * (x1 - x) + y;
	NxReal x2 = (outfl + 1.0f) * width;
	NxReal y2 = slope * (x2 - x) + y;
	if (y1 < ymin) {
		x1 = (ymin - y) / slope + x;
		y1 = ymin;
	}
	else if (y1 > ymax) {
		x1 = (ymax - y) / slope + x;
		y1 = ymax;
	}
	if (y2 < ymin) {
		x2 = (ymin - y) / slope + x;
		y2 = ymin;
	}
	else if (y2 > ymax) {
		x2 = (ymax - y) / slope + x;
		y2 = ymax;
	}
	return Line2D(x1, y1, x2, y2);
}

static Line2D linePoint(Vec2 p1, Vec2 p2, NxReal width = 640, NxReal height = 480)
{
	NxReal dx = p1.x - p2.x;
	NxReal dy = p1.y - p2.y;
	NxReal xm = 0.5f * (p1.x + p2.x);
	NxReal ym = 0.5f * (p1.y + p2.y);

	return linePoint(width, height, xm, ym, dy, - dx);
}

NxReal distanceSquared(Vec2 a, Vec2 b)
{
	NxReal dx = a.x - b.x;
	NxReal dy = a.y - b.y;
	return dx * dx + dy * dy;
}

//Use Fortune's algorithm for n log n (see netlib sweep2) in the future
QVector<Edge> getDelaunayEdges(QVector<Vec2> points)
{

	QVector<Edge> edges;

	int nofp = points.size();
	for (int i = 0; i < nofp - 1; i++) {
		Vec2 p1 = points[i];
		for (int j = i + 1; j < nofp; j++) {
			Vec2 p2 = points[j];
			Line2D line1 = linePoint(p1, p2);  //trans
			double dxba = line1.getX2() - line1.getX1();
			double dyba = line1.getY2() - line1.getY1();
			bool intsw = true;
			for (int k = 0; k < nofp; k++) {
				if (k != i && k != j && intsw) {
					Vec2 p3 = points[k];
					Line2D line2 = linePoint(p1, p3);   //trans
					double dx21 = line2.getX2() - line2.getX1();
					double dy21 = line2.getY2() - line2.getY1();
					double sw1
						= (dy21 * (line2.getX2() - line1.getX1())
						- dx21 * (line2.getY2() - line1.getY1()));
					double sw2 = dy21 * dxba - dx21 * dyba;
					if (abs(sw2) > 0.0f) {
						double sw3
							= (dx21 * (p1.y - line2.getY1())
							- dy21 * (p1.x - line2.getX1()));
						double sw4
							= sw3 * (dx21 * (line1.getY1() - line2.getY1())
							- dy21 * (line1.getX1()
							- line2.getX1()));
						double sw5
							= sw3 * (dx21 * (line1.getY2() - line2.getY1())
							- dy21 * (line1.getX2()
							- line2.getX1()));
						sw1 /= sw2;
						double xt = line1.getX1() + sw1 * dxba;
						double yt = line1.getY1() + sw1 * dyba;
						if (sw4 > 0.0 && sw5 < 0.0)
							line1.setLine(line1.getX1(), line1.getY1(), xt,
							yt);
						else if (sw4 < 0.0 && sw5 > 0.0)
							line1.setLine(xt, yt, line1.getX2(),
							line1.getY2());
						else if (sw4 < 0.0 && sw5 < 0.0)
							intsw = false;
					}
				}
			}
			Vec2 lp1 = line1.getP1();
			Vec2 lp2 = line1.getP2();
			for (int k = 0; k < nofp; k++) {
				if (k != i && k != j && intsw) {
					Vec2 p3 = points[k];
					int sw1
						= (int) (distanceSquared(p3,lp1) - distanceSquared(p1,lp1));
					int sw2
						= (int) (distanceSquared(p3,lp2) - distanceSquared(p1,lp2));
					if (sw1 < 0 || sw2 < 0)
						intsw = false;
				}
			}
			if (intsw)
			{
				Edge e;
				e.start = i;
				e.end = j;

				edges.append(e);
			}

		}
	}
	return edges;
}
