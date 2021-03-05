#include <vector>
#include "framework.h"
#include "Settings.h"
#include "Point3D.h"
#include "Triangle.h"
#include "Camera.h"
#include "EngineBase.h"

Triangle::Triangle(Point3D p0, Point3D p1, Point3D p2, Texture* t)
{
	points[0] = p0;
	points[1] = p1;
	points[2] = p2;
	cameraPoints[0] = p0;
	cameraPoints[1] = p1;
	cameraPoints[2] = p2;
	drawPoints[2] = p2;
	drawPoints[0] = p0;
	drawPoints[1] = p1;
	drawPoints[2] = p2;
	texture = t;
}

void Triangle::CalculateWorldPoints(Point3D position, Point3D rotation)
{
	// Rotate
	for (int i = 0; i < 3; i++)
		worldPoints[i] = EngineBase::Rotate(points[i], rotation);

	// Translate
	for (int i = 0; i < 3; i++)
		worldPoints[i] = EngineBase::Translate(worldPoints[i], position);
}

void Triangle::CalculateCameraView(Camera* camera)
{
	// Translate
	Point3D cameraPos = camera->GetPosition();
	Point3D translateBy = Point3D::Create(-cameraPos.x, -cameraPos.y, -cameraPos.z);
	for (int i = 0; i < 3; i++)
		cameraPoints[i] = EngineBase::Translate(worldPoints[i], translateBy);

	// Rotate
	Point3D cameraRot = camera->GetRotation();
	Point3D rotateBy = Point3D::Create(-cameraRot.x, -cameraRot.y, -cameraRot.z);
	for (int i = 0; i < 3; i++)
		cameraPoints[i] = EngineBase::Rotate(cameraPoints[i], rotateBy);

	// Average Z (for sorting triangles)
	averageZ = (cameraPoints[0].z + cameraPoints[1].z + cameraPoints[2].z) / 3.0;
}

void Triangle::CalculateDrawPoints()
{
	// Apply perspective
	for (int i = 0; i < 3; i++)
		drawPoints[i] = EngineBase::ApplyPerspective(cameraPoints[i]);

	// Center screen
	for (int i = 0; i < 3; i++)
		drawPoints[i] = EngineBase::CenterScreen(drawPoints[i]);

	// Normal Z -  is the triangle facing us or the other way
	normalZ = (drawPoints[1].x - drawPoints[0].x) * (drawPoints[2].y - drawPoints[0].y) - (drawPoints[1].y - drawPoints[0].y) * (drawPoints[2].x - drawPoints[0].x);
}

void Triangle::Draw(int* renderBuffer)
{
	
	Point3D aux;
	if (drawPoints[0].y > drawPoints[1].y) {
		aux = drawPoints[0];
		drawPoints[0] = drawPoints[1];
		drawPoints[1] = aux;
	}
	if (drawPoints[0].y > drawPoints[2].y) {
		aux = drawPoints[0];
		drawPoints[0] = drawPoints[2];
		drawPoints[2] = aux;
	}
	if (drawPoints[1].y > drawPoints[2].y) {
		aux = drawPoints[1];
		drawPoints[1] = drawPoints[2];
		drawPoints[2] = aux;
	}

	int p0x = drawPoints[0].x;
	int p0y = drawPoints[0].y;
	int p1x = drawPoints[1].x;
	int p1y = drawPoints[1].y;
	int p2x = drawPoints[2].x;
	int p2y = drawPoints[2].y;

	int texWidth = texture->GetWidth();
	int texHeight = texture->GetHeight();

	if (p0y < p1y) {
		double slope1 = ((double)p1x - p0x) / (p1y - p0y);
		double slope2 = ((double)p2x - p0x) / (p2y - p0y);
		for (int i = 0; i <= p1y - p0y; i++) {
			int x1 = p0x + i * slope1;
			int x2 = p0x + i * slope2;
			int y = p0y + i;

			double us = drawPoints[0].u + ((double)y - p0y) / (p1y - p0y) * (drawPoints[1].u - drawPoints[0].u);
			double vs = drawPoints[0].v + ((double)y - p0y) / (p1y - p0y) * (drawPoints[1].v - drawPoints[0].v);
			double ws = drawPoints[0].w + ((double)y - p0y) / (p1y - p0y) * (drawPoints[1].w - drawPoints[0].w);

			double ue = drawPoints[0].u + ((double)y - p0y) / (p2y - p0y) * (drawPoints[2].u - drawPoints[0].u);
			double ve = drawPoints[0].v + ((double)y - p0y) / (p2y - p0y) * (drawPoints[2].v - drawPoints[0].v);
			double we = drawPoints[0].w + ((double)y - p0y) / (p2y - p0y) * (drawPoints[2].w - drawPoints[0].w);

			if (x1 > x2) {
				int aux = x1;
				x1 = x2;
				x2 = aux;
				double aux2 = us;
				us = ue;
				ue = aux2;
				aux2 = vs;
				vs = ve;
				ve = aux2;
				aux2 = ws;
				ws = we;
				we = aux2;
			}
			if (x2 > x1) {
				double u = us * texWidth;
				double ustep = (ue - us) / (x2 - x1) * texWidth;
				double v = vs * texHeight;
				double vstep = (ve - vs) / (x2 - x1) * texHeight;
				double w = ws;
				double wstep = (we - ws) / (x2 - x1);
				for (int x = x1; x <= x2; x++) {
					u += ustep;
					v += vstep;
					w += wstep;
					renderBuffer[RESOLUTION_X * y + x] = texture->GetValue(u / w, v / w);
				}
			}
		}
	}
	if (p1y < p2y) {
		double slope1 = ((double)p2x - p1x) / (p2y - p1y);
		double slope2 = ((double)p2x - p0x) / (p2y - p0y);
		double sx = p2x - (p2y - p1y) * slope2;
		for (int i = 0; i <= p2y - p1y; i++) {
			int x1 = p1x + i * slope1;
			int x2 = sx + i * slope2;
			int y = p1y + i;

			double us = drawPoints[1].u + ((double)y - p1y) / (p2y - p1y) * (drawPoints[2].u - drawPoints[1].u);
			double vs = drawPoints[1].v + ((double)y - p1y) / (p2y - p1y) * (drawPoints[2].v - drawPoints[1].v);
			double ws = drawPoints[1].w + ((double)y - p1y) / (p2y - p1y) * (drawPoints[2].w - drawPoints[1].w);

			double ue = drawPoints[0].u + ((double)y - p0y) / (p2y - p0y) * (drawPoints[2].u - drawPoints[0].u);
			double ve = drawPoints[0].v + ((double)y - p0y) / (p2y - p0y) * (drawPoints[2].v - drawPoints[0].v);
			double we = drawPoints[0].w + ((double)y - p0y) / (p2y - p0y) * (drawPoints[2].w - drawPoints[0].w);

			if (x1 > x2) {
				int aux = x1;
				x1 = x2;
				x2 = aux;
				double aux2 = us;
				us = ue;
				ue = aux2;
				aux2 = vs;
				vs = ve;
				ve = aux2;
				aux2 = ws;
				ws = we;
				we = aux2;
			}
			if (x2 > x1) {
				double u = us * texWidth;
				double ustep = (ue - us) / (x2 - x1) * texWidth;
				double v = vs * texHeight;
				double vstep = (ve - vs) / (x2 - x1) * texHeight;
				double w = ws;
				double wstep = (we - ws) / (x2 - x1);
				for (int x = x1; x <= x2; x++) {
					u += ustep;
					v += vstep;
					w += wstep;
					renderBuffer[RESOLUTION_X * y + x] = texture->GetValue(u / w, v / w);
				}
			}
		}
	}

}

bool Triangle::SortOrder(Triangle* triangle1, Triangle* triangle2)
{
	return triangle1->averageZ > triangle2->averageZ;
}

double Triangle::GetNormalZ()
{
	return normalZ;
}

Point3D* Triangle::GetCameraPoints()
{
	return cameraPoints;
}

Point3D* Triangle::GetDrawPoints()
{
	return drawPoints;
}

std::list<Triangle*> Triangle::GetZClippedTriangles()
{
	std::list<Triangle*> toReturn;
	toReturn.push_back(new Triangle(cameraPoints[0], cameraPoints[1], cameraPoints[2], texture));

	int noTriangles;
	std::vector<Point3D> insidePoints;
	std::vector<Point3D> outsidePoints;

	// Z
	noTriangles = toReturn.size();
	for (int i = 0; i < noTriangles; i++) {

		Triangle* currentTriangle = toReturn.front();
		toReturn.pop_front();

		insidePoints.clear();
		outsidePoints.clear();

		bool pointsAreOutside[3];
		for (int i = 0; i < 3; i++)
		{
			pointsAreOutside[i] = currentTriangle->GetCameraPoints()[i].z < 0;
			if (pointsAreOutside[i])
				outsidePoints.push_back(currentTriangle->GetCameraPoints()[i]);
			else
				insidePoints.push_back(currentTriangle->GetCameraPoints()[i]);
		}
		delete currentTriangle;
		if (outsidePoints.size() == 0)
		{
			toReturn.push_back(new Triangle(insidePoints[0], insidePoints[1], insidePoints[2], texture));
		}
		else if (outsidePoints.size() == 1)
		{
			Point3D extraPoint1;
			extraPoint1.x = outsidePoints[0].x + (0 - outsidePoints[0].z) * (insidePoints[0].x - outsidePoints[0].x) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.y = outsidePoints[0].y + (0 - outsidePoints[0].z) * (insidePoints[0].y - outsidePoints[0].y) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.z = 0;
			extraPoint1.u = outsidePoints[0].u + (0 - outsidePoints[0].z) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.v = outsidePoints[0].v + (0 - outsidePoints[0].z) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.w = outsidePoints[0].w + (0 - outsidePoints[0].z) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].z - outsidePoints[0].z);

			Point3D extraPoint2;
			extraPoint2.x = outsidePoints[0].x + (0 - outsidePoints[0].z) * (insidePoints[1].x - outsidePoints[0].x) / (insidePoints[1].z - outsidePoints[0].z);
			extraPoint2.y = outsidePoints[0].y + (0 - outsidePoints[0].z) * (insidePoints[1].y - outsidePoints[0].y) / (insidePoints[1].z - outsidePoints[0].z);
			extraPoint2.z = 0;
			extraPoint2.u = outsidePoints[0].u + (0 - outsidePoints[0].z) * (insidePoints[1].u - outsidePoints[0].u) / (insidePoints[1].z - outsidePoints[0].z);
			extraPoint2.v = outsidePoints[0].v + (0 - outsidePoints[0].z) * (insidePoints[1].v - outsidePoints[0].v) / (insidePoints[1].z - outsidePoints[0].z);
			extraPoint2.w = outsidePoints[0].w + (0 - outsidePoints[0].z) * (insidePoints[1].w - outsidePoints[0].w) / (insidePoints[1].z - outsidePoints[0].z);

			if (pointsAreOutside[0]) {
				toReturn.push_back(new Triangle(extraPoint1, insidePoints[0], insidePoints[1], texture));
				toReturn.push_back(new Triangle(extraPoint2, extraPoint1, insidePoints[1], texture));
			} else if (pointsAreOutside[1]) {
				toReturn.push_back(new Triangle(extraPoint1, insidePoints[1], insidePoints[0], texture));
				toReturn.push_back(new Triangle(extraPoint2, extraPoint1, insidePoints[0], texture));
			} else if (pointsAreOutside[2]) {
				toReturn.push_back(new Triangle(extraPoint1, insidePoints[0], insidePoints[1], texture));
				toReturn.push_back(new Triangle(extraPoint2, extraPoint1, insidePoints[1], texture));
			}
		}
		else if (outsidePoints.size() == 2)
		{
			Point3D extraPoint1;
			extraPoint1.x = outsidePoints[0].x + (0 - outsidePoints[0].z) * (insidePoints[0].x - outsidePoints[0].x) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.y = outsidePoints[0].y + (0 - outsidePoints[0].z) * (insidePoints[0].y - outsidePoints[0].y) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.z = 0;
			extraPoint1.u = outsidePoints[0].u + (0 - outsidePoints[0].z) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.v = outsidePoints[0].v + (0 - outsidePoints[0].z) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].z - outsidePoints[0].z);
			extraPoint1.w = outsidePoints[0].w + (0 - outsidePoints[0].z) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].z - outsidePoints[0].z);

			Point3D extraPoint2;
			extraPoint2.x = outsidePoints[1].x + (0 - outsidePoints[1].z) * (insidePoints[0].x - outsidePoints[1].x) / (insidePoints[0].z - outsidePoints[1].z);
			extraPoint2.y = outsidePoints[1].y + (0 - outsidePoints[1].z) * (insidePoints[0].y - outsidePoints[1].y) / (insidePoints[0].z - outsidePoints[1].z);
			extraPoint2.z = 0;
			extraPoint2.u = outsidePoints[1].u + (0 - outsidePoints[1].z) * (insidePoints[0].u - outsidePoints[1].u) / (insidePoints[0].z - outsidePoints[1].z);
			extraPoint2.v = outsidePoints[1].v + (0 - outsidePoints[1].z) * (insidePoints[0].v - outsidePoints[1].v) / (insidePoints[0].z - outsidePoints[1].z);
			extraPoint2.w = outsidePoints[1].w + (0 - outsidePoints[1].z) * (insidePoints[0].w - outsidePoints[1].w) / (insidePoints[0].z - outsidePoints[1].z);

			if (!pointsAreOutside[0]) {
				toReturn.push_back(new Triangle(extraPoint2, insidePoints[0], extraPoint1, texture));
			} else if (!pointsAreOutside[1]) {
				toReturn.push_back(new Triangle(extraPoint1, insidePoints[0], extraPoint2, texture));
			} else if (!pointsAreOutside[2]) {
				toReturn.push_back(new Triangle(extraPoint2, insidePoints[0], extraPoint1, texture));
			}
			
		}
	}

	return toReturn;
}

std::list<Triangle*> Triangle::GetClippedTriangles()
{
	std::list<Triangle*> toReturn;
	toReturn.push_back(new Triangle(drawPoints[0], drawPoints[1], drawPoints[2], texture));

	int noTriangles;
	std::vector<Point3D> insidePoints;
	std::vector<Point3D> outsidePoints;

	// LEFT
	noTriangles = toReturn.size();
	for (int i = 0; i < noTriangles; i++) {

		Triangle* currentTriangle = toReturn.front();
		toReturn.pop_front();

		insidePoints.clear();
		outsidePoints.clear();
		for (int i = 0; i < 3; i++)
		{
			if (currentTriangle->GetDrawPoints()[i].x < 0)
				outsidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
			else
				insidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
		}
		delete currentTriangle;
		if (outsidePoints.size() == 0)
		{
			toReturn.push_back(new Triangle(insidePoints[0], insidePoints[1], insidePoints[2], texture));
		}
		else if (outsidePoints.size() == 1)
		{
			Point3D extraPoint1;
			extraPoint1.x = 0;
			extraPoint1.y = outsidePoints[0].y + (0 - outsidePoints[0].x) * (insidePoints[0].y - outsidePoints[0].y) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.z = outsidePoints[0].z + (0 - outsidePoints[0].x) * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.u = outsidePoints[0].u + (0 - outsidePoints[0].x) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.v = outsidePoints[0].v + (0 - outsidePoints[0].x) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.w = outsidePoints[0].w + (0 - outsidePoints[0].x) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].x - outsidePoints[0].x);

			Point3D extraPoint2;
			extraPoint2.x = 0;
			extraPoint2.y = outsidePoints[0].y + (0 - outsidePoints[0].x) * (insidePoints[1].y - outsidePoints[0].y) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.z = outsidePoints[0].z + (0 - outsidePoints[0].x) * (insidePoints[1].z - outsidePoints[0].z) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.u = outsidePoints[0].u + (0 - outsidePoints[0].x) * (insidePoints[1].u - outsidePoints[0].u) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.v = outsidePoints[0].v + (0 - outsidePoints[0].x) * (insidePoints[1].v - outsidePoints[0].v) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.w = outsidePoints[0].w + (0 - outsidePoints[0].x) * (insidePoints[1].w - outsidePoints[0].w) / (insidePoints[1].x - outsidePoints[0].x);

			toReturn.push_back(new Triangle(extraPoint1, insidePoints[0], insidePoints[1], texture));

			toReturn.push_back(new Triangle(extraPoint2, extraPoint1, insidePoints[1], texture));
		}
		else if (outsidePoints.size() == 2)
		{
			Point3D extraPoint1;
			extraPoint1.x = 0;
			extraPoint1.y = outsidePoints[0].y + (0 - outsidePoints[0].x) * (insidePoints[0].y - outsidePoints[0].y) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.z = outsidePoints[0].z + (0 - outsidePoints[0].x) * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.u = outsidePoints[0].u + (0 - outsidePoints[0].x) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.v = outsidePoints[0].v + (0 - outsidePoints[0].x) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.w = outsidePoints[0].w + (0 - outsidePoints[0].x) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].x - outsidePoints[0].x);

			Point3D extraPoint2;
			extraPoint2.x = 0;
			extraPoint2.y = outsidePoints[1].y + (0 - outsidePoints[1].x) * (insidePoints[0].y - outsidePoints[1].y) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.z = outsidePoints[1].z + (0 - outsidePoints[1].x) * (insidePoints[0].z - outsidePoints[1].z) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.u = outsidePoints[1].u + (0 - outsidePoints[1].x) * (insidePoints[0].u - outsidePoints[1].u) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.v = outsidePoints[1].v + (0 - outsidePoints[1].x) * (insidePoints[0].v - outsidePoints[1].v) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.w = outsidePoints[1].w + (0 - outsidePoints[1].x) * (insidePoints[0].w - outsidePoints[1].w) / (insidePoints[0].x - outsidePoints[1].x);

			toReturn.push_back(new Triangle(extraPoint1, extraPoint2, insidePoints[0], texture));
		}
	}

	// RIGHT
	noTriangles = toReturn.size();
	for (int i = 0; i < noTriangles; i++) {

		Triangle* currentTriangle = toReturn.front();
		toReturn.pop_front();

		insidePoints.clear();
		outsidePoints.clear();
		for (int i = 0; i < 3; i++)
		{
			if (currentTriangle->GetDrawPoints()[i].x >= RESOLUTION_X)
				outsidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
			else
				insidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
		}
		delete currentTriangle;
		if (outsidePoints.size() == 0)
		{
			toReturn.push_back(new Triangle(insidePoints[0], insidePoints[1], insidePoints[2], texture));
		}
		else if (outsidePoints.size() == 1)
		{
			Point3D extraPoint1;
			extraPoint1.x = RESOLUTION_X - 1;
			extraPoint1.y = outsidePoints[0].y + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].y - outsidePoints[0].y) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.z = outsidePoints[0].z + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.u = outsidePoints[0].u + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.v = outsidePoints[0].v + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.w = outsidePoints[0].w + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].x - outsidePoints[0].x);

			Point3D extraPoint2;
			extraPoint2.x = RESOLUTION_X - 1;
			extraPoint2.y = outsidePoints[0].y + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[1].y - outsidePoints[0].y) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.z = outsidePoints[0].z + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[1].z - outsidePoints[0].z) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.u = outsidePoints[0].u + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[1].u - outsidePoints[0].u) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.v = outsidePoints[0].v + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[1].v - outsidePoints[0].v) / (insidePoints[1].x - outsidePoints[0].x);
			extraPoint2.w = outsidePoints[0].w + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[1].w - outsidePoints[0].w) / (insidePoints[1].x - outsidePoints[0].x);

			toReturn.push_back(new Triangle(extraPoint1, insidePoints[0], insidePoints[1], texture));

			toReturn.push_back(new Triangle(extraPoint2, extraPoint1, insidePoints[1], texture));
		}
		else if (outsidePoints.size() == 2)
		{
			Point3D extraPoint1;
			extraPoint1.x = RESOLUTION_X - 1;
			extraPoint1.y = outsidePoints[0].y + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].y - outsidePoints[0].y) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.z = outsidePoints[0].z + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.u = outsidePoints[0].u + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.v = outsidePoints[0].v + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].x - outsidePoints[0].x);
			extraPoint1.w = outsidePoints[0].w + (RESOLUTION_X - 1 - outsidePoints[0].x) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].x - outsidePoints[0].x);

			Point3D extraPoint2;
			extraPoint2.x = RESOLUTION_X - 1;
			extraPoint2.y = outsidePoints[1].y + (RESOLUTION_X - 1 - outsidePoints[1].x) * (insidePoints[0].y - outsidePoints[1].y) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.z = outsidePoints[1].z + (RESOLUTION_X - 1 - outsidePoints[1].x) * (insidePoints[0].z - outsidePoints[1].z) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.u = outsidePoints[1].u + (RESOLUTION_X - 1 - outsidePoints[1].x) * (insidePoints[0].u - outsidePoints[1].u) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.v = outsidePoints[1].v + (RESOLUTION_X - 1 - outsidePoints[1].x) * (insidePoints[0].v - outsidePoints[1].v) / (insidePoints[0].x - outsidePoints[1].x);
			extraPoint2.w = outsidePoints[1].w + (RESOLUTION_X - 1 - outsidePoints[1].x) * (insidePoints[0].w - outsidePoints[1].w) / (insidePoints[0].x - outsidePoints[1].x);

			toReturn.push_back(new Triangle(extraPoint1, extraPoint2, insidePoints[0], texture));
		}
	}

	// TOP
	noTriangles = toReturn.size();
	for (int i = 0; i < noTriangles; i++) {

		Triangle* currentTriangle = toReturn.front();
		toReturn.pop_front();

		insidePoints.clear();
		outsidePoints.clear();
		for (int i = 0; i < 3; i++)
		{
			if (currentTriangle->GetDrawPoints()[i].y < 0)
				outsidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
			else
				insidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
		}
		delete currentTriangle;
		if (outsidePoints.size() == 0)
		{
			toReturn.push_back(new Triangle(insidePoints[0], insidePoints[1], insidePoints[2], texture));
		}
		else if (outsidePoints.size() == 1)
		{
			Point3D extraPoint1;
			extraPoint1.x = outsidePoints[0].x - outsidePoints[0].y * (insidePoints[0].x - outsidePoints[0].x) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.y = 0;
			extraPoint1.z = outsidePoints[0].z - outsidePoints[0].y * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.u = outsidePoints[0].u - outsidePoints[0].y * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.v = outsidePoints[0].v - outsidePoints[0].y * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.w = outsidePoints[0].w - outsidePoints[0].y * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].y - outsidePoints[0].y);

			Point3D extraPoint2;
			extraPoint2.x = outsidePoints[0].x - outsidePoints[0].y * (insidePoints[1].x - outsidePoints[0].x) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.y = 0;
			extraPoint2.z = outsidePoints[0].z - outsidePoints[0].y * (insidePoints[1].z - outsidePoints[0].z) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.u = outsidePoints[0].u - outsidePoints[0].y * (insidePoints[1].u - outsidePoints[0].u) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.v = outsidePoints[0].v - outsidePoints[0].y * (insidePoints[1].v - outsidePoints[0].v) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.w = outsidePoints[0].w - outsidePoints[0].y * (insidePoints[1].w - outsidePoints[0].w) / (insidePoints[1].y - outsidePoints[0].y);

			toReturn.push_back(new Triangle(extraPoint1, insidePoints[0], insidePoints[1], texture));

			toReturn.push_back(new Triangle(extraPoint2, extraPoint1, insidePoints[1], texture));
		}
		else if (outsidePoints.size() == 2)
		{
			Point3D extraPoint1;
			extraPoint1.x = outsidePoints[0].x - outsidePoints[0].y * (insidePoints[0].x - outsidePoints[0].x) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.y = 0;
			extraPoint1.z = outsidePoints[0].z - outsidePoints[0].y * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.u = outsidePoints[0].u - outsidePoints[0].y * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.v = outsidePoints[0].v - outsidePoints[0].y * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.w = outsidePoints[0].w - outsidePoints[0].y * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].y - outsidePoints[0].y);

			Point3D extraPoint2;
			extraPoint2.x = outsidePoints[1].x - outsidePoints[1].y * (insidePoints[0].x - outsidePoints[1].x) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.y = 0;
			extraPoint2.z = outsidePoints[1].z - outsidePoints[1].y * (insidePoints[0].z - outsidePoints[1].z) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.u = outsidePoints[1].u - outsidePoints[1].y * (insidePoints[0].u - outsidePoints[1].u) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.v = outsidePoints[1].v - outsidePoints[1].y * (insidePoints[0].v - outsidePoints[1].v) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.w = outsidePoints[1].w - outsidePoints[1].y * (insidePoints[0].w - outsidePoints[1].w) / (insidePoints[0].y - outsidePoints[1].y);

			toReturn.push_back(new Triangle(extraPoint1, extraPoint2, insidePoints[0], texture));
		}
	}

	// BOTTOM
	noTriangles = toReturn.size();
	for (int i = 0; i < noTriangles; i++) {

		Triangle* currentTriangle = toReturn.front();
		toReturn.pop_front();

		insidePoints.clear();
		outsidePoints.clear();
		for (int i = 0; i < 3; i++)
		{
			if (currentTriangle->GetDrawPoints()[i].y >= RESOLUTION_Y)
				outsidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
			else
				insidePoints.push_back(currentTriangle->GetDrawPoints()[i]);
		}
		delete currentTriangle;
		if (outsidePoints.size() == 0)
		{
			toReturn.push_back(new Triangle(insidePoints[0], insidePoints[1], insidePoints[2], texture));
		}
		else if (outsidePoints.size() == 1)
		{
			Point3D extraPoint1;
			extraPoint1.x = outsidePoints[0].x + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].x - outsidePoints[0].x) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.y = RESOLUTION_Y - 1;
			extraPoint1.z = outsidePoints[0].z + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.u = outsidePoints[0].u + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.v = outsidePoints[0].v + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.w = outsidePoints[0].w + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].y - outsidePoints[0].y);

			Point3D extraPoint2;
			extraPoint2.x = outsidePoints[0].x + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[1].x - outsidePoints[0].x) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.y = RESOLUTION_Y - 1;
			extraPoint2.z = outsidePoints[0].z + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[1].z - outsidePoints[0].z) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.u = outsidePoints[0].u + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[1].u - outsidePoints[0].u) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.v = outsidePoints[0].v + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[1].v - outsidePoints[0].v) / (insidePoints[1].y - outsidePoints[0].y);
			extraPoint2.w = outsidePoints[0].w + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[1].w - outsidePoints[0].w) / (insidePoints[1].y - outsidePoints[0].y);

			toReturn.push_back(new Triangle(extraPoint1, insidePoints[0], insidePoints[1], texture));

			toReturn.push_back(new Triangle(extraPoint2, extraPoint1, insidePoints[1], texture));
		}
		else if (outsidePoints.size() == 2)
		{
			Point3D extraPoint1;
			extraPoint1.x = outsidePoints[0].x + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].x - outsidePoints[0].x) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.y = RESOLUTION_Y - 1;
			extraPoint1.z = outsidePoints[0].z + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].z - outsidePoints[0].z) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.u = outsidePoints[0].u + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].u - outsidePoints[0].u) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.v = outsidePoints[0].v + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].v - outsidePoints[0].v) / (insidePoints[0].y - outsidePoints[0].y);
			extraPoint1.w = outsidePoints[0].w + (RESOLUTION_Y - 1 - outsidePoints[0].y) * (insidePoints[0].w - outsidePoints[0].w) / (insidePoints[0].y - outsidePoints[0].y);

			Point3D extraPoint2;
			extraPoint2.x = outsidePoints[1].x + (RESOLUTION_Y - 1 - outsidePoints[1].y) * (insidePoints[0].x - outsidePoints[1].x) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.y = RESOLUTION_Y - 1;
			extraPoint2.z = outsidePoints[1].z + (RESOLUTION_Y - 1 - outsidePoints[1].y) * (insidePoints[0].z - outsidePoints[1].z) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.u = outsidePoints[1].u + (RESOLUTION_Y - 1 - outsidePoints[1].y) * (insidePoints[0].u - outsidePoints[1].u) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.v = outsidePoints[1].v + (RESOLUTION_Y - 1 - outsidePoints[1].y) * (insidePoints[0].v - outsidePoints[1].v) / (insidePoints[0].y - outsidePoints[1].y);
			extraPoint2.w = outsidePoints[1].w + (RESOLUTION_Y - 1 - outsidePoints[1].y) * (insidePoints[0].w - outsidePoints[1].w) / (insidePoints[0].y - outsidePoints[1].y);

			toReturn.push_back(new Triangle(extraPoint1, extraPoint2, insidePoints[0], texture));
		}
	}

	
	return toReturn;
}