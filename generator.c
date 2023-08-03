#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef size_t usize;

typedef double f64;

static f64 Clamp(f64 minimum, f64 x, f64 maximum)
{
	if (x < minimum)
	{
		return minimum;
	}

	if (x > maximum)
	{
		return maximum;
	}

	return x;
}

static f64 RandomInRange(f64 minimum, f64 maximum)
{
	u64 randomBytes = 0;
	arc4random_buf(&randomBytes, sizeof(randomBytes));
	f64 range = maximum - minimum;
	return (randomBytes & ((1l << 53) - 1)) * ldexp(1, -53) * range +
	       minimum;
}

static void RandomRangeInRange(
        f64 minimum, f64 maximum, f64 *outMinimum, f64 *outMaximum)
{
	f64 range = maximum - minimum;
	f64 radius = RandomInRange(range / 32, range / 4);
	f64 center = RandomInRange(minimum + radius, maximum - radius);
	*outMinimum = center - radius;
	*outMaximum = center + radius;
	*outMinimum = Clamp(minimum, *outMinimum, maximum);
	*outMaximum = Clamp(minimum, *outMaximum, maximum);
}

typedef struct
{
	f64 x0;
	f64 y0;
	f64 x1;
	f64 y1;
} Pair;

static Pair GeneratePair(f64 xMinimum, f64 xMaximum, f64 yMinimum, f64 yMaximum)
{
	return (Pair){
		.x0 = RandomInRange(xMinimum, xMaximum),
		.y0 = RandomInRange(yMinimum, yMaximum),
		.x1 = RandomInRange(xMinimum, xMaximum),
		.y1 = RandomInRange(yMinimum, yMaximum),
	};
}

static f64 Square(f64 x)
{
	return x * x;
}

static f64 DegreesToRadians(f64 degrees)
{
	return 0.01745329251994329577 * degrees;
}

static f64 HaversineDistance(f64 x0, f64 y0, f64 x1, f64 y1, f64 earthRadius)
{
	f64 latitudeRange = DegreesToRadians(y1 - y0);
	f64 longtitudeRange = DegreesToRadians(x1 - x0);
	f64 latitude0 = DegreesToRadians(y0);
	f64 latitude1 = DegreesToRadians(y1);

	f64 rootTerm = Square(sin(latitudeRange / 2.0)) +
	               cos(latitude0) * cos(latitude1) *
	                       Square(sin(longtitudeRange / 2));

	return 2 * earthRadius * asin(sqrt(rootTerm));
}

int main(void)
{
	usize pairCount = 10000000;
	usize chunkCount = 10;
	usize chunkSize = pairCount / chunkCount;
	pairCount = chunkCount * chunkSize;

	Pair *pairs = calloc(pairCount, sizeof(Pair));

	f64 average = 0;

	for (usize i = 0; i < chunkCount; i++)
	{
		f64 xMinimum = 0;
		f64 xMaximum = 0;
		f64 yMinimum = 0;
		f64 yMaximum = 0;
		RandomRangeInRange(-180, 180, &xMinimum, &xMaximum);
		RandomRangeInRange(-90, 90, &yMinimum, &yMaximum);

		for (usize j = 0; j < chunkSize; j++)
		{
			Pair pair = GeneratePair(
			        xMinimum, xMaximum, yMinimum, yMaximum);
			f64 distance = HaversineDistance(
			        pair.x0, pair.y0, pair.x1, pair.y1, 6372.8);
			pairs[i * chunkSize + j] = pair;
			average += distance;
		}
	}

	average /= pairCount;
	printf("Average Haversine distance: %f\n", average);

	FILE *output = fopen("output.json", "w");

	fprintf(output, "{\"pairs\":[\n");

	for (usize i = 0; i < pairCount; i++)
	{
		Pair pair = pairs[i];
		if (i != 0)
		{
			fprintf(output, ",\n");
		}
		fprintf(output,
		        "    {\"x0\":%.16f, \"y0\":%.16f, \"x1\":%.16f, "
		        "\"y1\":%.16f}",
		        pair.x0, pair.y0, pair.x1, pair.y1);
	}

	fprintf(output, "\n]}\n");

	fclose(output);
}
