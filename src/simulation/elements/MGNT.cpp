#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_MGNT()
{
	Identifier = "DEFAULT_PT_MGNT";
	Name = "MGNT";
	Colour = 0xA0A0C0_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f	* CFDS;
	Falldown = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 50;

	Weight = 100;

	HeatConduct = 251;
	Description = "Magnet. Attracts iron, metal, and powder particles. Loses magnetism above Curie temperature.";

	Properties = TYPE_SOLID|PROP_CONDUCTS|PROP_LIFE_DEC|PROP_HOT_GLOW;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	// Curie temperature for iron: ~1043K. Above this, MGNT loses magnetism and becomes BRMT (broken metal)
	HighTemperature = 1043.0f;
	HighTemperatureTransition = PT_BRMT;

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	auto &sd = SimulationData::CRef();
	auto &elements = sd.elements;

	// Magnetic field range: 5 pixels
	int range = 5;

	for (int rx = -range; rx <= range; rx++)
	{
		for (int ry = -range; ry <= range; ry++)
		{
			if (!rx && !ry)
				continue;
			if (!InBounds(x+rx, y+ry))
				continue;
			auto r = pmap[y+ry][x+rx];
			if (!r)
				continue;
			auto rt = TYP(r);

			// Attract ferromagnetic materials: IRON, BMTL, METL, BRMT, TUNG, GOLD
			// Also attract loose powders/particles: DUST, SAND, BREC, STNE, GUNP, BCOL, COAL
			bool isFerromagnetic = (rt == PT_IRON || rt == PT_BMTL || rt == PT_METL ||
			                        rt == PT_BRMT || rt == PT_TUNG);
			bool isMagneticPowder = (rt == PT_DUST || rt == PT_BREC || rt == PT_BCOL || rt == PT_COAL);

			if (isFerromagnetic || isMagneticPowder)
			{
				float dist2 = float(rx * rx + ry * ry);
				if (dist2 < 1.0f)
					dist2 = 1.0f;
				float dist = sqrtf(dist2);

				// Force follows inverse-square law, stronger for ferromagnetic
				float strength = isFerromagnetic ? 0.4f : 0.15f;
				float force = strength / dist2;

				// Cap the force to prevent instability
				if (force > 0.5f)
					force = 0.5f;

				// Apply attractive force toward the magnet
				parts[ID(r)].vx -= (float(rx) / dist) * force;
				parts[ID(r)].vy -= (float(ry) / dist) * force;
			}

			// MGNT + MGNT: like poles repel (using tmp as polarity)
			if (rt == PT_MGNT)
			{
				float dist2 = float(rx * rx + ry * ry);
				if (dist2 < 1.0f)
					dist2 = 1.0f;
				float dist = sqrtf(dist2);

				// Same polarity repels, different attracts
				// tmp stores polarity: 0 = north facing up, 1 = south facing up
				bool samePole = (parts[i].tmp == parts[ID(r)].tmp);
				float force = 0.15f / dist2;
				if (force > 0.3f)
					force = 0.3f;

				if (samePole)
				{
					// Repel
					parts[ID(r)].vx += (float(rx) / dist) * force;
					parts[ID(r)].vy += (float(ry) / dist) * force;
				}
			}

			// Magnets can induce SPRK in nearby conductors when moving fast
			if ((elements[rt].Properties & PROP_CONDUCTS) && parts[ID(r)].life == 0)
			{
				float mySpeed = sqrtf(parts[i].vx * parts[i].vx + parts[i].vy * parts[i].vy);
				if (mySpeed > 2.0f && sim->rng.chance(1, 20))
				{
					// Use create_part with the existing ID to change type, but set ctype to preserve the conductor
					int sparkId = ID(r);
					sim->part_change_type(sparkId, x + rx, y + ry, PT_SPRK);
					parts[sparkId].ctype = rt;
					parts[sparkId].life = 4; // Typical spark life
				}
			}
		}
	}

	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Polarity-dependent coloring: North=blueish, South=reddish
	if (cpart->tmp == 0)
	{
		// North pole - blue tint
		*colr = 120;
		*colg = 130;
		*colb = 200;
	}
	else
	{
		// South pole - red tint
		*colr = 200;
		*colg = 100;
		*colb = 110;
	}
	*pixel_mode |= PMODE_GLOW;
	return 0;
}
