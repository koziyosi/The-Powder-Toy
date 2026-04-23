#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_OOZE()
{
	Identifier = "DEFAULT_PT_OOZE";
	Name = "OOZE";
	Colour = 0x4EA54E_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.80f;
	Collision = 0.0f;
	Gravity = 0.15f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 30;

	Weight = 40;

	DefaultProperties.temp = R_TEMP - 2.0f + 273.15f;
	HeatConduct = 20;
	Description = "Non-Newtonian fluid. Flows slowly, but solidifies under pressure or fast impact (shear thickening).";

	Properties = TYPE_LIQUID|PROP_NEUTPENETRATE;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 573.0f; // Decomposes at ~300C
	HighTemperatureTransition = PT_FIRE;

	// tmp stores viscosity state: 0=liquid, higher=more solid
	// tmp2 stores color variation

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	// Calculate local shear rate (velocity differences with neighbors)
	float shearRate = 0.0f;
	int neighborCount = 0;

	for (int rx = -1; rx <= 1; rx++)
	{
		for (int ry = -1; ry <= 1; ry++)
		{
			if (!rx && !ry)
				continue;
			auto r = pmap[y+ry][x+rx];
			if (!r)
				continue;

			float dvx = parts[i].vx - parts[ID(r)].vx;
			float dvy = parts[i].vy - parts[ID(r)].vy;
			shearRate += sqrtf(dvx * dvx + dvy * dvy);
			neighborCount++;
		}
	}

	if (neighborCount > 0)
		shearRate /= neighborCount;

	// Also consider pressure
	float pressure = fabsf(sim->pv[y/CELL][x/CELL]);

	// Dilatancy: high shear rate or high pressure -> solidify
	float solidifyThreshold = 0.8f;
	float liquefyRate = 0.95f;

	if (shearRate > solidifyThreshold || pressure > 3.0f)
	{
		// Increase viscosity (solidify)
		if (parts[i].tmp < 100)
			parts[i].tmp += int((shearRate + pressure) * 5);
		if (parts[i].tmp > 100)
			parts[i].tmp = 100;
	}
	else
	{
		// Slowly relax back to liquid state
		parts[i].tmp = int(parts[i].tmp * liquefyRate);
		if (parts[i].tmp < 0)
			parts[i].tmp = 0;
	}

	// Apply viscosity effect: dampen velocity based on tmp (viscosity)
	float viscosityFactor = 1.0f - (parts[i].tmp / 200.0f);  // 0.5 to 1.0
	if (viscosityFactor < 0.5f)
		viscosityFactor = 0.5f;

	parts[i].vx *= viscosityFactor;
	parts[i].vy *= viscosityFactor;

	// When highly solidified (tmp > 70), behave more like a solid
	// Attract neighboring OOZE particles (cohesion / stickiness)
	if (parts[i].tmp > 50)
	{
		for (int rx = -1; rx <= 1; rx++)
		{
			for (int ry = -1; ry <= 1; ry++)
			{
				if (!rx && !ry)
					continue;
				auto r = pmap[y+ry][x+rx];
				if (!r)
					continue;
				if (TYP(r) == PT_OOZE)
				{
					float dx = parts[i].x - parts[ID(r)].x;
					float dy = parts[i].y - parts[ID(r)].y;
					float d2 = dx * dx + dy * dy;
					if (d2 > 1.5f)
					{
						float cohesion = (parts[i].tmp / 100.0f) * 0.3f;
						float nd = sqrtf(d2);
						parts[i].vx -= (dx / nd) * cohesion * 0.5f;
						parts[i].vy -= (dy / nd) * cohesion * 0.5f;
						parts[ID(r)].vx += (dx / nd) * cohesion * 0.5f;
						parts[ID(r)].vy += (dy / nd) * cohesion * 0.5f;
					}
				}
			}
		}
	}

	// Generate random color variation on creation
	if (parts[i].tmp2 == 0)
		parts[i].tmp2 = sim->rng.between(1, 30);

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Color shifts from green (liquid) to dark green-brown (solidified)
	int viscosity = cpart->tmp;
	int variation = cpart->tmp2;

	// Liquid state: bright green; solid state: dark brownish-green
	*colr = 40 + viscosity / 3 + variation;
	*colg = 170 - viscosity;
	*colb = 40 + variation;

	if (*colr > 255) *colr = 255;
	if (*colg < 40) *colg = 40;

	if (viscosity > 60)
		*pixel_mode |= PMODE_BLUR;

	return 0;
}
