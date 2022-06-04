#!/bin/awk -f
# This script attempts to generate usable data
# A lot of assumptions, extrapolations and simplifications are made: chiefly the lack of elliptical orbits.

BEGIN {
	FS = "\t"
}

!/^#/ {
	id = $1
	idname = $2
	name = $3
	alt = $4
	radius = $5
	pradius = $6
	mass = $7
	maxorb = $8
	minorb = $9
	ecc = $10
	inc = $11
	node = $12
	peri = $13
	anomoly = $14
	epoch = $15
	rotdays = $16
	orbdays = $17
	ephemeris = $18
	type = $19
	parent = $20

	if (type == "DB" || type == "DL") # not a body
		next

	if (name == "-")
		name = idname

	if (name == "Sun") {
		name = "Sol"
		alt = "Sun"
		minorb = 0
		maxorb = 0
		orbdays = 0
	}

	if (type == "*")  nicetype = "Star"
	if (type == "A")  nicetype = "Asteroid"
	if (type == "AS") nicetype = "Asteroid moon"
	if (type == "M")  nicetype = "Moon"
	if (type == "P")  nicetype = "Planet"

	if (type == "W") {
		if (parent == "-")
			nicetype = "Dwarf planet"
		else
			nicetype = "Moon"
	}

	if (type == "CP" || type == "C") nicetype = "Comet"

	if (parent == "EMB") {
		if (name == "Earth")
			parent = "-"
		else
			parent = "Earth"
	}

	if (parent == "-")
		parent = "Sol"

	if (mass ~ /\?$/)
		sub(/\?$/, "", mass)

	if (mass ~ /M$/) {
		sub(/M$/, "", mass)
		massmult = 1000000
	} else {
		massmult = 1
	}

	if (mass ~ /e+/) {
		mantissa = mass
		sub(/e+.*/, "", mantissa)
		exponent = mass
		sub(/.*e+/, "", exponent)
		mass = mantissa * (10 ^ exponent)
	}

	mass = mass * massmult

	if (minorb ~ /M$/) {
		sub(/M$/, "", minorb)
		minorb = minorb * 1000000
	}

	if (maxorb ~ /M$/) {
		sub(/M$/, "", maxorb)
		maxorb = maxorb * 1000000
	}

	if (minorb == 0)
		minorb = maxorb

	if (orbdays ~ /yr$/) {
		sub(/yr$/, "", orbdays)
		orbdays = orbdays * 365.25 # good enough
	}

	if (maxorb ~ /^[-?0]/)
		maxorb = minorb

	if (name != "Sol" && (radius ~ /^[-?0]$/ || maxorb ~ /^[-?0]$/ || mass ~ /^[-?0]$/ || orbdays ~ /^[-?0]$/))
		next

	added[name] = 1

	if (added[parent] != 1)
		next

	file = sprintf("sol/%s", name)
	system(sprintf("mkdir -p $(dirname \"%s\")", file))

	if (name != "Sol")
		printf("parent\t%s\n", parent) > file
	printf("type\t%s\n", nicetype) > file
	printf("radius\t%d\n", radius) > file
	printf("mass\t%d\n", mass) > file
	if (rotdays !~ /^[-?0]$/)
		printf("rotdays\t%f\n", rotdays) > file
	printf("orbdays\t%f\n", orbdays) > file

	if (nicetype == "Comet") {
		printf("mindist\t%d\n", minorb) > file
		printf("maxdist\t%d\n", maxorb) > file
		printf("curdist\t%d\n", (rand() * (maxorb - minorb)) + minorb) > file
	} else {
		printf("dist\t%d\n", (minorb + maxorb) / 2) > file
	}

	printf("curtheta\t%d\n", rand() * 360) > file
}
