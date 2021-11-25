#include "RayTracer.h"

#include <chrono>
#include <cmath>

#include "Texture.h"
#include "Scene.h"
#include "Camera.h"
#include "Light.h"
#include "Material.h"

namespace GraphicsEngine
{
	Vector3 Sanitize(const Vector3& contribution)
	{
		return Vector3(std::max(0.f, contribution.X), std::max(0.f, contribution.Y), std::max(0.f, contribution.Z));
	}

	float Sanitize(float value)
	{
		return std::max(0.f, value);
	}

	Vector3 ValidateData(const Vector3& contribution, const char* file, int line)
	{
		if (contribution.X < 0 || contribution.Y < 0 || contribution.Z < 0)
			std::cout << "[" << file << ":" << line << "] WARNING: negative value in vector '" << contribution << "'" << std::endl;

		return Sanitize(contribution);
	}

	float ValidateData(float value, const char* file, int line)
	{
		if (value < 0 || value < 0 || value < 0)
			std::cout << "[" << file << ":" << line << "] WARNING: negative value '" << value << "'" << std::endl;

		return Sanitize(value);
	}

	Pixel::Pixel(const RGBA& color)
	{
		R = color.R;
		G = color.G;
		B = color.B;
	}

	Pixel::operator RGBA()
	{
		return RGBA(R, G, B, 1);
	}

	RayTracer::~RayTracer()
	{
		if (Data != nullptr)
		{
			delete[] Data;
			delete[] Rays;
		}
	}

	void RayTracer::SetMaxThreads(int count)
	{
		if (count == MaxThreads)
			return;

		delete[] Threads;

		Threads = new Thread[count]();

		MaxThreads = count;
	}

	int RayTracer::GetMaxThreads() const
	{
		return MaxThreads;
	}

	void RayTracer::Configure(int width, int height)
	{
		if (width == Width && height == Height)
			return;

		if (Data != nullptr)
		{
			delete[] Data;
			delete[] Rays;
		}

		if (width != 0 && height != 0)
		{
			Data = new Pixel[width * height];
			Rays = new Vector[width * height];
		}
		else
		{
			Data = nullptr;
			Rays = nullptr;
		}

		Width = width;
		Height = height;

		SetViewport(0, 0, -1, -1);
	}

	void RayTracer::SetViewport(int minX, int minY, int maxX, int maxY)
	{
		MinX = std::max(0, minX);
		MinY = std::max(0, minY);
		MaxX = std::min(Width, maxX == -1 ? Width : maxX);
		MaxY = std::min(Height, maxY == -1 ? Height : maxY);

		int width = std::max(MaxX - MinX, 1);
		int height = std::max(MaxY - MinY, 1);

		if (width == Width && height == Height)
			return;

		if (Data != nullptr)
			delete[] Data;

		if (width != 0 && height != 0)
			Data = new Pixel[width * height];
		else
			Data = nullptr;
	}

	RayTracer::Batch RayTracer::GetNext()
	{
		BatchLock.lock();

		Batch batch;
		do
		{
			batch = Batch{ NextX, NextY };

			if (NextY >= MaxY)
				break;

			NextX += BatchWidth;

			if (NextX >= Width)
			{
				NextX = 0;
				NextY += BatchHeight;
			}
		}
		while (batch.X >= MaxX || batch.Y >= MaxY || batch.X + BatchWidth <= MinX || batch.Y + BatchHeight <= MinY);

		BatchLock.unlock();

		return batch;
	}

	float pow4(float x)
	{
		x *= x;

		return x * x;
	}

	void RayTracer::Render()
	{
		ThisScene = CurrentScene.lock();

		if (ThisScene == nullptr)
			return;

		GlobalLight = ThisScene->GlobalLight.lock();
		ThisCamera = ThisScene->CurrentCamera.lock();

		if (ThisCamera == nullptr)
			return;

		AtmosphereRadius = PlanetRadius + AtmosphereHeight;
		AtmosphereRadiusSquared = AtmosphereRadius * AtmosphereRadius;
		PlanetRadiusSquared = PlanetRadius * PlanetRadius;
		PlanetPosition = (PlanetRadius + Altitude) * PlanetDirection;
		AtmosphereStep = 1 / float(AtmosphereSteps);
		OpticalDepthStep = 1 / float(OpticalDepthSteps);
		ScatterCoefficients = ScatterStrength * Vector3(pow4(400 / Wavelengths.X), pow4(400 / Wavelengths.Y), pow4(400 / Wavelengths.Z));

		if (AtmosphereFollowsCamera)
			PlanetPosition += ThisCamera->GetTransformation().Translation();

		PlanetPosition.W = 1;

		if (RayWidth != Width || RayHeight != Height)
		{
			RayWidth = Width;
			RayHeight = Height;

			Vector3 dimensions = ThisCamera->GetDimensions();

			for (int x = 0; x < Width; ++x)
			{
				for (int y = 0; y < Height; ++y)
				{
					Vector3 direction = Vector3(
						dimensions.X * (float(x) - float(0.5f * Width)) / Width,
						dimensions.Y * (float(y) - float(0.5f * Height)) / Height,
						-ThisCamera->GetProjectionPlane()
					).Unit();

					Rays[x + y * Width] = Vector(direction.X, direction.Y, direction.Z);
				}
			}
		}

		for (int i = 0; i < MaxThreads; ++i)
		{
			Threads[i].Finished = true;
			Threads[i].ThreadTotal = std::chrono::nanoseconds(0);
		}

		if (!Initialized)
		{
			Initialized = true;
			Accelerator.Accelerate(CurrentScene.lock());
		}
		else
		{
			Accelerator.UpdateMaterials();
		}

		RayCount = 0;
		BounceRayCount = 0;
		ShadowRayCount = 0;

		FailedLights = 0;
		LightsChecked = 0;
		BouncesChecked = 0;

		RunningThreads = 0;
		MaxRunningThreads = 0;

		NextX = 0;
		NextY = 0;

		auto start = std::chrono::high_resolution_clock::now();

		if (ComputeAsync)
		{
			Batch batch = GetNext();

			while (batch.Y < Height)
			{
				bool dispatched = false;

				while (!dispatched)
				{
					for (int i = 0; i < MaxThreads && !dispatched; ++i)
					{
						if (Threads[i].Finished)
						{
							Threads[i].Spawn(this, batch.X, batch.Y, i);

							dispatched = true;
						}
					}
				}

				batch = GetNext();
			}
		}
		else
			for (int i = 0; i < MaxThreads; ++i)
				Threads[i].SpawnSync(this, i);

		for (int i = 0; i < MaxThreads; ++i)
			Threads[i].Free(this);

		if (DisplayOutput)
		{
			auto stop = std::chrono::high_resolution_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(stop - start);

			float durationMS = (float(duration.count()) / 1e6f);

			std::cout << "time: " << durationMS << " miliseconds" << std::endl;

			std::cout << "total rays: " << RayCount + ShadowRayCount << std::endl;
			std::cout << "bounce rays: " << BounceRayCount << std::endl;
			std::cout << "shadow rays: " << ShadowRayCount << std::endl;

			std::cout << "failed light checks: " << FailedLights << std::endl;
			std::cout << "lights in range: " << LightsChecked - FailedLights << std::endl;
			std::cout << "bounces checked: " << BouncesChecked << std::endl;

			std::cout << "peak concurrent threads: " << MaxRunningThreads << std::endl;

			for (int i = 0; i < MaxThreads; ++i)
			{
				float duration = Threads[i].ThreadTotal.count() / 1e6f;

				std::cout << "thread[" << i << "] uptime: " << (duration / durationMS) << std::endl;
			}
		}
	}

	void RayTracer::DrawTo(const std::shared_ptr<Texture>& output) const
	{
		int width = std::max(MaxX - MinX, 1);
		int height = std::max(MaxY - MinY, 1);

		if (output->GetTextureID() == 0)
			output->Load(width, height);

		output->Upload(width, height, Data, GL_FLOAT, GL_RGB32F, GL_RGB);
	}

	void RayTracer::Save(const std::string& filePath) const
	{

	}

	int RayTracer::GetIndex(int x, int y) const
	{
		int width = MaxX - MinX;

		return x - MinX + (y - MinY) * width;
	}

	const Pixel& RayTracer::GetPixel(int x, int y) const
	{

		return Data[GetIndex(x, y)];
	}

	Pixel& RayTracer::GetPixel(int x, int y)
	{
		return Data[GetIndex(x, y)];
	}

	void RayTracer::SetPixel(int x, int y, const Pixel& pixel)
	{
		Data[GetIndex(x, y)] = pixel;
	}

	Ray RayTracer::GetRay(int x, int y) const
	{
		return Ray(
			ThisCamera->GetTransformation().Translation(),
			ThisCamera->GetTransformation() * Vector3(Rays[x + y * Width])
		);
	}

	Vector3 RayTracer::ComputeLightFilter(const Vector3& color, const Vector3& filter) const
	{
		Vector3 filterColor = Vector3(color).Scale(Vector3(1, 1, 1, 0));
		Vector3 filterTint = (1 - color.W) * Vector3(1, 1, 1) + color.W * filterColor;

		return Vector3(filter).Scale((1 - color.W) * filterTint);
	}

	void RayTracer::Cast(const RayInfo& info, const Ray& ray, float length, const ResultsCallback& callback, const ResultsCallback& filter, IndexVector& stack) const
	{
		Accelerator.CastRay(ray, length, callback, filter, stack, info.ObjectNode, info.NodeID);
	}

	struct ShadowCastData
	{
		int LastHit = -1;
		int LastHitTriangle = -1;
	};

	Vector3 RayTracer::ComputeShadowFilter(const Ray& ray, float length, RayInfo& info, const ShadowCastData& shadowData, IndexVector& stack) const
	{
		Vector3 filter(1, 1, 1);

		auto resultsProcessorLambda = [this, &ray, &filter](const CastResults& results) -> RayFilterResults
		{
			return RayFilterResults::Ignore;
		};

		auto resultsFilterLambda = [&filter, &info, this, &shadowData](const CastResults& results) -> RayFilterResults
		{
			if (results.Color.W == 0 || (shadowData.LastHitTriangle == results.FaceIndex))
				return RayFilterResults::Ignore;

			if (results.Color.W == 1)
			{

				filter = Vector3();

				info = RayInfo{
					results.ObjectNode,
					results.NodeID,
					results.FaceIndex
				};

				return RayFilterResults::Stop;
			}

			filter = ComputeLightFilter(results.Color, filter);

			return RayFilterResults::Ignore;
		};

		Cast(info, ray, length, std::ref(resultsProcessorLambda), std::ref(resultsFilterLambda), stack);

		if (filter != Vector3(0, 0, 0))
			info = RayInfo{ -1, -1 };

		return filter;
	}

	Vector3 RayTracer::RefractVector(const Vector3& vector, const Vector3& normal, float dot, float materialIndex, float incidenceIndex) const
	{
		float scale = 1;

		if (dot < 0)
			dot *= -1;
		else
		{
			std::swap(incidenceIndex, materialIndex);
			scale = -1;
		}

		float ratio = incidenceIndex / materialIndex;
		float sine = 1 - ratio * ratio * (1 - dot * dot);

		if (sine > 0)
			return ratio * vector + scale * (ratio * dot - std::sqrtf(sine)) * normal;

		return Vector3();
	}

	Vector3 RayTracer::ReflectVector(const Vector3& vector, const Vector3& normal) const
	{
		return vector - 2 * (vector * normal) * normal;
	}

	Vector3 RayTracer::ReflectVector(float dot, const Vector3& vector, const Vector3& normal) const
	{
		return 2 * dot * normal - vector;
	}

	float RayTracer::Fresnel(float dot, float internalRefractiveIndex, float externalRefractiveIndex) const
	{
		if (dot > 0)
			std::swap(internalRefractiveIndex, externalRefractiveIndex);

		float sine = externalRefractiveIndex / internalRefractiveIndex * std::sqrtf(std::max(0.f, 1 - dot * dot));

		if (sine >= 1)
			return 1;

		float refractedCosine = std::sqrtf(std::max(0.f, 1 - sine * sine));

		dot = std::abs(dot);

		float reflectedPerpendicular = (internalRefractiveIndex * dot - externalRefractiveIndex * refractedCosine) / (internalRefractiveIndex * dot + externalRefractiveIndex * refractedCosine);
		float reflectedParallel = (externalRefractiveIndex * dot - internalRefractiveIndex * refractedCosine) / (externalRefractiveIndex * dot + internalRefractiveIndex * refractedCosine);

		return 0.5f * (reflectedPerpendicular * reflectedPerpendicular + reflectedParallel * reflectedParallel);
	}

	float RayTracer::NormalDistribution(float reflectedDot, float roughness) const
	{
		float denominator = reflectedDot * reflectedDot * (roughness - 1) + 1;

		if (std::abs(denominator) < 1e-9f)
			denominator = 1;

		return roughness / (PI * denominator * denominator);
	}

	float RayTracer::GeometryShadowing(float vectorDot, float roughness) const
	{
		float denominator = vectorDot * (1 - roughness) + roughness;

		return vectorDot / std::max(denominator, 0.0001f);
	}

	float RayTracer::GeometryContribution(float lookDot, float lightDot, float roughness) const
	{
		return GeometryShadowing(lookDot, (roughness + 1) * (roughness + 1) / 8) * GeometryShadowing(lightDot, roughness * roughness / 2);
	}

	float RayTracer::Largest(const Vector3& vector) const
	{
		return std::max(std::max(vector.X, vector.Y), vector.Z);
	}

	float RayTracer::Smallest(const Vector3& vector) const
	{
		return std::min(std::min(vector.X, vector.Y), vector.Z);
	}

	Vector3 RayTracer::SampleNormal(FloatRNG& rng, const Vector3& normal, const Vector3& view, float roughness) const
	{
		bool normalOnX = std::abs(normal.X) > 0.8f;

		Vector3 baseAxis = Vector3(float(!normalOnX), 0, float(normalOnX));
		Vector3 axis1 = normal.Cross(baseAxis).Unit();
		Vector3 axis2 = normal.Cross(axis1);

		Vector3 localView = Vector3(axis1 * view, normal * view, axis2 * view);
		Vector3 stretchedView = Vector3(localView.X * roughness, localView.Y, localView.Z * roughness).Unit();

		Vector3 viewAxis1 = (stretchedView.Y < 0.999f) ? stretchedView.Cross(Vector3(0, 1, 0)).Unit() : Vector3(1, 0, 0);
		Vector3 viewAxis2 = viewAxis1.Cross(stretchedView);

		float random1 = rng.Next();
		float random2 = rng.Next();

		float a = 1.f / (1.f + stretchedView.Y);
		float r = std::sqrtf(random1);
		float phi = (random2 < a) ? random2 / a * PI : PI + (random2 - a) / (1 - a) * PI;

		float p1 = r * std::cosf(phi);
		float p2 = r * std::sinf(phi) * ((random2 < a) ? 1 : stretchedView.Y);

		Vector3 stretchedResult = p1 * viewAxis1 + p2 * viewAxis2 + std::sqrtf(std::max(0.f, 1.f - p1 * p1 - p2 * p2)) * stretchedView;
		Vector3 result = Vector3(roughness * stretchedResult.X, std::max(stretchedResult.Y, 0.f), roughness * stretchedResult.Z).Unit();

		return result.X * axis1 + result.Y * normal + result.Z * axis2;
	}

	Vector3 RayTracer::ComputeReflectionFilter(const Vector3& direction, const Vector3& reflected, const Vector3& normal, const Vector3& microfacetNormal, const Vector3& baseReflectivity, float roughnessSquared) const
	{
		float lightDot = reflected * normal;

		if (lightDot <= 0)
			return Vector3();

		float halfLookDot = microfacetNormal * direction;

		float fresnelExponential = std::powf(1 - halfLookDot, 5);
		Vector3 fresnel = CHECK_VALUE(baseReflectivity + fresnelExponential * (Vector3(1, 1, 1) - baseReflectivity));

		float lookDot = std::max(0.f, direction * normal);

		float lookShadowing = std::sqrtf(roughnessSquared + (1 - roughnessSquared) * lookDot * lookDot);

		float denomA = lookDot * std::sqrtf(roughnessSquared + (1 - roughnessSquared) * lightDot * lightDot);
		float denomB = lightDot * lookShadowing;
		float denomC = lookShadowing + lookDot;

		float geometryShadowing = CHECK_VALUE(2 * lookDot * lightDot / (denomA + denomB));
		float geometry = CHECK_VALUE(2 * lookDot / denomC);

		return CHECK_VALUE((geometryShadowing / geometry) * fresnel);
	}

	Vector3 RayTracer::BaseReflectivity(float metalness, float refractiveIndex, float externalIndex, const Vector3& color, float dot) const
	{
		float fresnel = Fresnel(dot, refractiveIndex, externalIndex);

		return (1 - metalness) * Vector3(fresnel, fresnel, fresnel) + metalness * Vector3(color.X, color.Y, color.Z);
	}

	Vector3 RayTracer::FresnelSchlick(float reflectedDot, const Vector3& baseReflectivity) const
	{
		return baseReflectivity + (Vector3(1, 1, 1) - baseReflectivity) * std::powf(1 - reflectedDot, 5);
	}

	Vector3 RayTracer::CookTorranceBRDF(const LightingParameters& parameters, float reflectedDot, float lightDot) const
	{
		float normalDistribution = NormalDistribution(reflectedDot, parameters.QuadRoughness);
		float geometryContribution = GeometryShadowing(parameters.LookDot, parameters.LookRoughness) * GeometryShadowing(lightDot, parameters.LightRoughness);
		Vector3 fresnelSchlick = FresnelSchlick(reflectedDot, parameters.BaseReflectivity);

		float denominator = std::max(4 * parameters.LookDot * lightDot, 0.0001f);

		Vector3 diffuse = (Vector3(1, 1, 1) - fresnelSchlick).Scale(parameters.Diffuse);
		Vector3 specular = ((normalDistribution * geometryContribution) / denominator) * fresnelSchlick;

		return diffuse + specular;
	}

	Vector3 RayTracer::GetReflectionFilter(const Material* material, float reflectedDot, float lookDot, float lightDot, const Vector3& baseReflectivity) const
	{
		float normalDistribution = 1 / ((0.4f + material->Roughness) * std::sqrtf(2 * PI));
		float geometryContribution = GeometryContribution(lookDot, lightDot, material->Roughness);
		Vector3 fresnelSchlick = FresnelSchlick(lookDot, baseReflectivity);

		return normalDistribution * geometryContribution * fresnelSchlick;
	}

	Vector3 RayTracer::GetRefractionFilter(const Material* material, float lightDot, const Vector3& color, const Vector3& baseReflectivity, const Vector3& lightFilter) const
	{
		return (lightDot * material->Transparency) * (Vector3(1, 1, 1) - FresnelSchlick(lightDot, baseReflectivity)).Scale(ComputeLightFilter(Vector3(color.X, color.Y, color.Z, 1 - material->Transparency), lightFilter));
	}

	bool SmallerThan(const Vector3& vector, float value)
	{
		return (vector.X < value) | (vector.Y < value) | (vector.Z < value);
	}

	Vector3 RayTracer::ComputeLighting(const LightingParameters& parameters, LightHandle light, const Vector3& lightFilter, RayInfo& info, const ShadowCastData& shadowData, Thread& thread) const
	{
		Vector3 lightDirection = -Vector3(light->Direction);
		float lightDistance = 10000;
		float lightScale = light->Brightness;

		if (light->Type != Enum::LightType::Directional)
		{
			lightDirection = Vector3(light->Position) - parameters.Intersection;

			lightDistance = lightDirection.SquareLength();

			if (light->Range < lightDistance)
			{
				if (PerformAnalytics)
					++thread.FailedLights;

				return Vector3();
			}

			lightScale *= light->Radius / lightDistance;

			lightDistance = std::sqrtf(lightDistance);

			lightDirection *= 1 / lightDistance;

			if (light->Type == Enum::LightType::Spot)
			{
				float spotlightDot = -(lightDirection * Vector3(light->Direction));

				if (spotlightDot < light->OuterRadius)
					return Vector3();

				if (spotlightDot < light->InnerRadius)
					lightScale *= float(std::pow((spotlightDot - light->OuterRadius) / (light->InnerRadius - light->OuterRadius), light->SpotlightFalloff));
			}
		}

		float lightDot = std::max(lightDirection * parameters.Normal, 0.f);

		if (lightDot <= 0)
			return Vector3();

		float reflectedDot = std::max((lightDirection + parameters.Look).Unit() * parameters.Normal, 0.f);

		Vector3 lightContribution = CookTorranceBRDF(parameters, reflectedDot, lightDot);

		lightContribution = (lightScale * lightDot) * lightContribution.Scale(light->Color).Scale(lightFilter);

		if (SmallerThan(lightContribution, 1e-7f))
			return Vector3();

		if (PerformAnalytics)
			++thread.ShadowRayCount;

		Vector3 shadowFilter = ComputeShadowFilter(Ray(parameters.Intersection, lightDirection), lightDistance, info, shadowData, thread.RayStack);

		return lightContribution.Scale(shadowFilter);
	}

	struct RayCastData
	{
		int LastHitTriangle = -1;
		int LastHitObject = -1;
		int Bounces = 0;
		const RayTracer::QueuedRay& RayData;
		Vector3 Illumination;
		Vector3 LightFilter = Vector3(1, 1, 1, 1);
		bool HitObject = false;
	};

	void RayTracer::CastRay(RayInfo& info, const Ray& ray, const RayCastData& rayData, const CastResultsCallbackWrapper& callback, IndexVector& stack) const
	{
		float finalDistance = rayData.RayData.Length;
		float closestDistance = rayData.RayData.Length;
		CastResults closest;

		auto resultsQueueLambda = [&ray, &rayData, &closestDistance, &closest, &info](const CastResults& results) -> RayFilterResults
		{
			closestDistance = results.Distance;
			closest = results;

			info.ObjectNode = results.ObjectNode;
			info.NodeID = results.NodeID;
			info.FaceIndex = results.FaceIndex;

			return RayFilterResults::Confirm;
		};

		auto resultsFilterLambda = [&finalDistance, &rayData, &closestDistance](const CastResults& results) -> RayFilterResults
		{
			if (results.Color.W == 0 || results.Distance > closestDistance || (rayData.LastHitTriangle == results.FaceIndex && rayData.LastHitObject == results.ObjectID))
				return RayFilterResults::Ignore;

			finalDistance = results.Distance;

			return RayFilterResults::Confirm;
		};

		Cast(info, Ray(ray.Start, rayData.RayData.Data.Direction), rayData.RayData.Length, std::ref(resultsQueueLambda), std::ref(resultsFilterLambda), stack);

		callback(closest);
	}

	RayTracer::SphereIntersection RayTracer::ComputeAtmosphereIntersection(const Ray& ray) const
	{
		Vector3 offset = PlanetPosition - ray.Start;

		float dot = ray.Direction * offset;

		float distanceSquared = offset * offset - dot * dot;

		if (distanceSquared > AtmosphereRadiusSquared)
			return SphereIntersection{ -1, -1 };

		float root = std::sqrtf(AtmosphereRadiusSquared - distanceSquared);

		float closest = dot - root;
		float farthest = dot + root;

		if (farthest < 0)
			return SphereIntersection{ -1, -1 };

		float surface = -1;

		if (distanceSquared < PlanetRadiusSquared)
		{
			float root = std::sqrtf(PlanetRadiusSquared - distanceSquared);

			float closest = dot - root;
			float farthest = dot + root;

			if (farthest > 0)
				surface = std::max(closest, 0.f);
		}

		return SphereIntersection{ std::max(closest, 0.f), std::max(farthest, 0.f), surface };
	}

	float RayTracer::ComputeSunRayLength(LightHandle light, const Vector3& position) const
	{
		Vector3 offset = PlanetPosition - position;

		float dot = -(Vector3(light->Direction) * offset);

		float distanceSquared = offset * offset - dot * dot;

		if (distanceSquared > AtmosphereRadiusSquared)
			return 0;

		float root = std::sqrtf(AtmosphereRadiusSquared - distanceSquared);

		return std::max(dot + root, 0.f);
	}

	float RayTracer::ComputeOpticalDepth(const Vector3& scatterPoint, const Vector3& direction, float rayLength) const
	{
		float opticalDepth = 0;
		float step = rayLength * OpticalDepthStep;

		for (float t = 0; t < rayLength; t += step)
		{
			float density = ComputeAtmosphericDensity(scatterPoint - t * direction);

			opticalDepth += density * step;
		}

		return opticalDepth;
	}

	float RayTracer::ComputeAtmosphericDensity(const Vector3& scatterPoint) const
	{
		float height = (scatterPoint - PlanetPosition).Length() - PlanetRadius;
		float relativeHeight = std::min(std::max(height / (AtmosphereRadius - PlanetRadius), 0.f), 1.f);

		return std::expf(-relativeHeight * AtmosphericDensityFalloff) * (1 - relativeHeight);
	}

	Vector3 RayTracer::ComputeAtmosphericLight(LightHandle light, const Ray& ray, const SphereIntersection& intersection) const
	{
		if (intersection.Exit == -1)
			return Vector3();
		
		float end = intersection.Exit;
		
		if (intersection.Surface != -1)
			end = intersection.Surface;
		
		float distance = end - intersection.Entry;
		Vector3 scatterPoint;
		
		Vector3 scatteredLight = 0;
		float scatterPointStep = distance * AtmosphereStep;
		
		for (float t = intersection.Entry; t < end; t += scatterPointStep)
		{
			scatterPoint = ray.Start + t * ray.Direction;

			float sunRayLength = ComputeSunRayLength(light, scatterPoint);
			float sunOpticalDepth = ComputeOpticalDepth(scatterPoint, light->Direction, sunRayLength);
			float viewOpticalDepth = ComputeOpticalDepth(scatterPoint, ray.Direction, t - intersection.Entry);
			Vector3 transmittance(
				std::expf(-(sunOpticalDepth + viewOpticalDepth) * ScatterCoefficients.X),
				std::expf(-(sunOpticalDepth + viewOpticalDepth) * ScatterCoefficients.Y),
				std::expf(-(sunOpticalDepth + viewOpticalDepth) * ScatterCoefficients.Z)
			);
			float localDensity = ComputeAtmosphericDensity(scatterPoint);

			scatteredLight += (localDensity * scatterPointStep) * transmittance.Scale(ScatterCoefficients);
		}

		return (light->Brightness) * Vector3(light->Color).Scale(scatteredLight);
	}

	Vector3 RayTracer::ProcessRay(const QueuedRay& rayData, Thread& thread, const Vector3& lightFilter, int bounces, int rayID) const
	{
		if (bounces >= MaxBounces)
			return Vector3();

		if (PerformAnalytics)
		{
			++thread.RayCount;

			if (bounces > 0)
				++thread.BounceRayCount;
		}

		int cacheSize = int(thread.RayCache.size());

		if (cacheSize <= rayID)
		{
			thread.RayCache.resize(std::max(cacheSize, rayID + 1));
			thread.RayShadowCache.resize(int(thread.RayCache.size()) * (GetLights() + 1));
		}

		RayCastData castData{ rayData.LastFace, rayData.LastObject, bounces, rayData, Vector3(), lightFilter };

		auto resultsProcessorLambda = [this, &castData, &thread, bounces, rayID, &rayData](const CastResults& results)
		{
			if (results.Color.W == 0)
				return;

			castData.HitObject = true;

			thread.RayCache[rayID] = RayInfo{
				results.ObjectNode,
				results.NodeID,
				results.FaceIndex
			};

			float normalScale = 1;

			if (!results.HitFront)
				normalScale *= -1;

			Vector3 normal = normalScale * results.Normal.Unit();

			float dot = castData.RayData.Data.Direction * normal;
			float incidenceIndex = results.MaterialProperties->RefractiveIndex;
			float externalIndex = 1;

			if (!results.HitFront)
				std::swap(incidenceIndex, externalIndex);

			Vector3 color = Vector3(results.Color).Scale(results.MaterialProperties->Albedo);
			Vector3 baseReflectivity = BaseReflectivity(results.MaterialProperties->Metalness, incidenceIndex, externalIndex, color, -1);

			float roughness = results.MaterialProperties->Roughness;
			float squaredRoughness = roughness * roughness;

			if (results.HitFront)
				castData.Illumination += CHECK_VALUE((results.MaterialProperties->Emission * Vector3(color.X, color.Y, color.Z)).Scale(castData.LightFilter));

			{
				int shadowCacheOffset = rayID * (GetLights() + 1);

				ShadowCastData shadowData{ results.ObjectID, thread.RayCache[rayID].FaceIndex };

				LightingParameters parameters{ std::max(-dot, 0.f), results.MaterialProperties, results.Intersection, normal, color, ((1 - results.MaterialProperties->Transparency) / PI) * parameters.Color, -castData.RayData.Data.Direction, baseReflectivity, (roughness + 1) * (roughness + 1) / 8, squaredRoughness / 2, squaredRoughness * squaredRoughness };

				for (int i = GetLight(-1) != nullptr ? -1 : 0; i < GetLights(); ++i)
					castData.Illumination += ComputeLighting(parameters, GetLight(i), castData.LightFilter, thread.RayShadowCache[shadowCacheOffset + i + 1], shadowData, thread);

				if (PerformAnalytics)
				{
					thread.LightsChecked += (GetLight(-1) != nullptr ? 1 : 0) + (long long)(GetLights());
					thread.BouncesChecked += 1;
				}
			}

			if (bounces + 1 < MaxBounces)
			{
				int samples = std::max(1, Samples / (bounces + 1));
				float weight = 1;

				{
					Vector3 microfacetNormal;

					if (UseMicrofacetNormals)
						microfacetNormal = SampleNormal(thread.RNG, normal, -castData.RayData.Data.Direction, roughness);
					else
						microfacetNormal = normal;

					Vector3 reflected = ReflectVector(castData.RayData.Data.Direction, microfacetNormal);

					Vector3 reflectionFilter = ComputeReflectionFilter(-castData.RayData.Data.Direction, reflected, normal, microfacetNormal, baseReflectivity, squaredRoughness).Scale(castData.LightFilter);
					reflectionFilter = CHECK_VALUE(reflectionFilter);

					if (Largest(reflectionFilter) > 0)
					{
						Ray bounceRay(results.Intersection, reflected);
						QueueRay(thread, QueuedRay{ castData.Bounces + 1, 2 * rayID + 1, thread.RayCache[rayID].FaceIndex, results.ObjectID, weight * reflectionFilter, bounceRay, rayData.Length - results.Distance });
					}

					if (results.Color.W < 1)
					{
						Vector3 refracted = RefractVector(castData.RayData.Data.Direction, microfacetNormal, castData.RayData.Data.Direction * microfacetNormal, incidenceIndex, externalIndex).Unit();

						float halfLookDot = -(microfacetNormal * refracted);
						float fresnel = std::max(0.f, 1 - Fresnel(halfLookDot, incidenceIndex, externalIndex));

						Vector3 refractionFilter = fresnel * (1 - results.Color.W) * (Vector3(1, 1, 1) - results.Color.W * Vector3(1 - results.Color.X, 1 - results.Color.Y, 1 - results.Color.Z)).Scale(castData.LightFilter);
						refractionFilter = CHECK_VALUE(refractionFilter);

						if (Largest(refractionFilter) > 0)
						{
							Ray refractedRay(results.Intersection, refracted);
							QueueRay(thread, QueuedRay{ castData.Bounces + 1, 2 * rayID + 2, thread.RayCache[rayID].FaceIndex, results.ObjectID, weight * refractionFilter, refractedRay, rayData.Length - results.Distance });
						}
					}
				}
			}

			return;
		};

		CastRay(thread.RayCache[rayID], rayData.Data, castData, std::ref(resultsProcessorLambda), thread.RayStack);

		if (!castData.HitObject)
		{
			if (RenderAtmosphere)
			{
				SphereIntersection intersection = ComputeAtmosphereIntersection(castData.RayData.Data);

				for (int i = GetDirectionalLight(-1) != nullptr ? -1 : 0; i < GetDirectionalLights(); ++i)
					castData.Illumination += ComputeAtmosphericLight(GetDirectionalLight(i), castData.RayData.Data, intersection).Scale(castData.LightFilter);
			}
			else if (RenderNaiveAtmosphere)
				castData.Illumination += ComputeAtmosphericLightNaive(castData.RayData.Data.Direction).Scale(castData.LightFilter);
		}

		return castData.Illumination;
	}

	Vector3 RayTracer::ComputeAtmosphericLightNaive(const Vector3& direction) const
	{
		const Vector3 skyColor (9.f / 255.f, 49.f / 255.f, 143.f / 255.f);
		const Vector3 horizonColor(190.f / 255.f, 200.f / 255.f, 215.f / 255.f);

		if (direction.Y > 0)
			return (1 - direction.Y) * horizonColor + direction.Y * skyColor;
		
		return (1 - direction.Y * direction.Y) * horizonColor;
	}

	void RayTracer::QueueRay(Thread& thread, const QueuedRay& ray) const
	{
		thread.QueuedRays.push_back(ray);
	}

	void RayTracer::ProcessBatch(int startX, int startY, int threadID)
	{
		Threads[threadID].RayCount = 0;
		Threads[threadID].BounceRayCount = 0;

		int increment = 8;

		for (int y = std::max(0, MinY - startY); y < BatchHeight && y + startY < MaxY; ++y)
		{
			for (int x = std::max(0, MinX - startX); x < BatchWidth && x + startX < MaxX; ++x)
			{
				int px = startX + x;
				int py = startY + y;

				if (px >= MaxX || py >= MaxY)
					continue;

				Ray ray = GetRay(px, py);

				float weight = 1 / float(Samples);

				Vector3 illumination;

				for (int i = 0; i < Samples; ++i)
				{
					QueueRay(Threads[threadID], QueuedRay{ 0, 0, -1, -1, Vector3(weight, weight, weight), ray, ThisCamera->GetFarPlane() });

					for (int i = 0; i < int(Threads[threadID].QueuedRays.size()); ++i)
					{
						const QueuedRay& ray = Threads[threadID].QueuedRays[i];

						illumination += CHECK_VALUE(ProcessRay(ray, Threads[threadID], ray.LightFilter, ray.Bounces, ray.RayID));
					}

					Threads[threadID].QueuedRays.clear();
				}

				Pixel& pixel = GetPixel(px, py);

				pixel.R = illumination.X;
				pixel.G = illumination.Y;
				pixel.B = illumination.Z;
			}
		}

		Threads[threadID].Finished = true;
	}

	int RayTracer::GetLights() const
	{
		return Accelerator.GetLights();
	}

	RayTracer::LightHandle RayTracer::GetLight(int index) const
	{
		return Accelerator.GetLight(index);
	}

	int RayTracer::GetDirectionalLights() const
	{
		return Accelerator.GetDirectionalLights();
	}

	RayTracer::LightHandle RayTracer::GetDirectionalLight(int index) const
	{
		return Accelerator.GetDirectionalLight(index);
	}

	RayTracer::Thread::~Thread()
	{
		Free();
	}

	void RayTracer::Thread::Spawn(RayTracer* rayTracer, int x, int y, int threadID)
	{
		Free(rayTracer);

		Allocated = true;
		Finished = false;

		FailedLights = 0;
		LightsChecked = 0;

		RayCount = 0;
		BounceRayCount = 0;
		BouncesChecked = 0;
		ShadowRayCount = 0;
		
		++rayTracer->RunningThreads;
		rayTracer->MaxRunningThreads = std::max(rayTracer->MaxRunningThreads, rayTracer->RunningThreads);

		new (ThreadData) std::thread([this](RayTracer* tracer, int nextX, int nextY, int threadID)
			{
				ThreadStarted = std::chrono::high_resolution_clock::now();
			}, rayTracer, x, y, threadID);

		rayTracer->ProcessBatch(x, y, threadID);
	}

	void RayTracer::Thread::SpawnSync(RayTracer* rayTracer, int threadID)
	{
		Free(rayTracer);

		Allocated = true;
		Finished = false;

		FailedLights = 0;
		LightsChecked = 0;

		RayCount = 0;
		BounceRayCount = 0;
		BouncesChecked = 0;
		ShadowRayCount = 0;

		++rayTracer->RunningThreads;
		rayTracer->MaxRunningThreads = std::max(rayTracer->MaxRunningThreads, rayTracer->RunningThreads);

		new (ThreadData) std::thread([this](RayTracer* tracer, int threadID)
			{
				ThreadStarted = std::chrono::high_resolution_clock::now();

				for (Batch batch = tracer->GetNext(); batch.Y < tracer->MaxY; batch = tracer->GetNext())
					tracer->ProcessBatch(batch.X, batch.Y, threadID);
			}, rayTracer, threadID);
	}

	void RayTracer::Thread::Free(RayTracer* rayTracer)
	{
		if (!Allocated)
			return;

		ThreadData->join();
		ThreadData->~thread();

		auto stop = std::chrono::high_resolution_clock::now();

		ThreadTotal += std::chrono::duration_cast<std::chrono::nanoseconds>(stop - ThreadStarted);

		rayTracer->FailedLights += FailedLights;
		rayTracer->LightsChecked += LightsChecked;

		rayTracer->RayCount += RayCount;
		rayTracer->BounceRayCount += BounceRayCount;
		rayTracer->BouncesChecked += BouncesChecked;
		rayTracer->ShadowRayCount += ShadowRayCount;

		--rayTracer->RunningThreads;

		Allocated = false;
		Finished = true;
	}
}