#pragma once
#include "Lib.h"
#include "MyModel.h"


struct RayInfo
{
	glm::vec3 origin;
	glm::vec3 Direction;
	float maxDistance;
	RayInfo(glm::vec3 start, glm::vec3 dir, float dis) :origin(start), Direction(dir), maxDistance(dis) {};
};
struct hitInfo
{
	bool isHit;
	glm::vec3 hitPos;
	float HitDis;
	pModel hitModel;
	hitInfo(bool isHit, glm::vec3 hitPos, float hitDis, pModel hitModel) :isHit(isHit), hitPos(hitPos), HitDis(hitDis), hitModel(hitModel) {};
};

bool Intersect(const RayInfo& ray, const glm::vec3& pA, const glm::vec3& pB, const glm::vec3& pC, glm::mat4 M2W, glm::vec3& hitPos, float& hitDis) //返回世界空间下的交点
{
	glm::mat4 iM2W = inverse(M2W);
	glm::vec3 o = glm::vec3(iM2W* glm::vec4(ray.origin, 1));
	glm::vec3 d = glm::vec3(iM2W*glm::vec4(ray.Direction, 0));
	glm::vec3 faceNormal = glm::cross(pA - pB, pB - pC);
	//glm::vec3 IntFaceNormal = dot(faceNormal, r.dirction) < 0 ? faceNormal : -faceNormal;  //两面的法线选择和入射光同一面的
	float denominator = glm::abs(dot(d, faceNormal));
	if (abs(denominator) < 0.0001) //平行的时候
	{
		return false;
	}
	else
	{
		//M-T方法
		//http://www.scratchapixel.com/lessons/3d-basic-rendering/ray-tracing-rendering-a-triangle/moller-trumbore-ray-triangle-intersection
		//通过重心坐标来进行射线检测
		glm::vec3 E1 = pB - pA;
		glm::vec3 E2 = pC - pA;
		glm::vec3 T = o - pA;
		glm::vec3 P = cross(d, E2);
		glm::vec3 Q = cross(T, E1);
		float PdotE1 = 1.0 / dot(P, E1);
		float t = dot(Q, E2) * PdotE1;
		float u = dot(P, T) * PdotE1;
		float v = dot(Q, d) * PdotE1;
		if (t < 0 || t>ray.maxDistance)
		{
			return false;
		}

		if (u>0 && v>0 && u + v<1)
		{
			hitPos = M2W* glm::vec4(o + t*d, 1);
			hitDis = length(hitPos - ray.origin);
			return true;

		}
		else
		{
			return false;
		}
	}
}

hitInfo RayCast(RayInfo ray, std::vector<pModel> models)
{
	glm::vec3 FirstHitPos;
	float FirstHitDis = 200;
	pModel hitModel;
	bool isHit = false;
	for (int i = 0; i < models.size(); i++)
	{
		std::vector<pMesh> meshs = models[i]->meshes;
		for (int j = 0; j < meshs.size(); j++)
		{
			pMesh mesh = meshs[j];
			for (int x = 0; x < meshs[j]->indices.size() - 2; x += 3)
			{
				glm::vec3 hitPos;
				float hitDis;
				glm::vec3 pA = mesh->vertices[mesh->indices[x]].position;
				glm::vec3 pB = mesh->vertices[mesh->indices[x + 1]].position;
				glm::vec3 pC = mesh->vertices[mesh->indices[x + 2]].position;
				if (Intersect(ray, pA, pB, pC, models[i]->M2W(), hitPos, hitDis) && hitDis<FirstHitDis)
				{
					isHit = true;
					FirstHitPos = hitPos;
					FirstHitDis = hitDis;
					hitModel = models[i];
				}
			}

		}
	}


	return hitInfo(isHit, FirstHitPos, FirstHitDis, hitModel);
}
