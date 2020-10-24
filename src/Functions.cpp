
#include <fstream>
#include <cmath>
#include <cstdint>

void save_image(const ::std::string &name, uint8_t* img_vals, int sizeX, int sizeY)
{
	auto as_ppm = [](const std::string &name) -> std::string {
		if (!((name.length() >= 4)
			&& (name.substr(name.length() - 4, 4) == ".ppm")))
		{
			return name + ".ppm";
		}
		else {
			return name;
		}
	};

	std::ofstream out(as_ppm(name), std::ios::out | std::ios::trunc);

	out << "P3\n" << sizeX << " " << sizeY << "\n255\n";
	const int N = (sizeX * sizeY) * 3;
	for (unsigned int i = 0; i < N; i += 3) {
		if (i % sizeX == 0)
			out << std::endl;

		const int r = int(img_vals[i]);
		const int g = int(img_vals[i + 1]);
		const int b = int(img_vals[i + 2]);
		out << r << " " << g << " " << b << "\t";
	}
	out.close();
}