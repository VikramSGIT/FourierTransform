#define _USE_MATH_DEFINES
#include <iostream>
#include <math.h>
#include <vector>
#include <complex>

#include <functional>
#include <unordered_map>
#include <glm/glm.hpp>

using vertex = glm::vec2;
using datatype = float;
using complex = std::complex<datatype>;

// Modified https://rextester.com/discussion/FLHE26043/Romberg-Integration to work with modern c++
// Romberg's Integration
template<typename T> std::complex<T> integrateC(const std::function<std::complex<T>(const T&)>& f, const T& a, const T& b, const T& err) {
    constexpr int max_steps = 20;

    T acc = err != 0 ? err : 1e-10;
    std::vector<std::vector<std::complex<T>>> R(2, std::vector<std::complex<T>>(max_steps + 1));
    T cErr = std::numeric_limits<T>().infinity();
    int cr = 0;
    T h = b - a;

    R[1 - cr][0] = (f(a) + f(b)) * static_cast<T>(h * 0.5);

    for (int i = 1; i <= max_steps; i++) {
        h /= 2;
        std::complex<T> c = 0;
        int ep = pow(2, (i - 1));

        for (int j = 1; j <= ep; j++)
            c += f(a + (j * 2 - 1) * h);

        R[cr][0] = c * h + R[1 - cr][0] / static_cast<T>(2);

        for (int j = 1; j <= i; j++) {
            T n_k = pow<T, T>(4, j);
            R[cr][j] = (R[cr][j - 1] * static_cast<T>(n_k) - R[1 - cr][j - 1]) / (n_k - 1);
        }

        cErr = abs(R[1 - cr][i - 1] - R[cr][i]);

        if (i > 1 && cErr < acc)
            return R[cr][i - 1];

        cr = 1 - cr;
    }

    return R[1 - cr][max_steps - 1];
}

void calc(const std::vector<vertex>& xy, uint32_t n, datatype error, std::vector<complex>& Cn) {
	std::vector<complex> data;
	auto it = xy.begin();

	float tau = 2 * M_PI;

	for (const vertex& ver : xy) data.push_back({ ver.x, ver.y });

	//f is frequency harmonics
	for (int64_t f = -static_cast<int64_t>(n); f <= n; f++) {
		complex C = integrateC<datatype>(std::bind([&data, &f](const datatype& t) -> complex {
			return data[t * (data.size() - 1)] * std::exp(complex{0, -f * 2 * static_cast<float>(M_PI * t)});
			}, std::placeholders::_1), 0, 1, error);
		Cn.push_back(C);
	}
}

std::vector<vertex> plot(const std::vector<complex>& Cn, datatype start, datatype end) {
	std::vector<vertex> res(std::abs(end - start));

	float omega = 2 * M_PI / std::abs(end - start);

    if (start > end) std::swap(start, end);

	for (size_t k = start; k < end; k++) {
		complex C = { 0, 0 };
		datatype n_har = -static_cast<float>(Cn.size() / 2);
		for (const complex& cn : Cn) {
			C += cn * std::exp(complex{ 0, n_har * omega * k});
			n_har++;
		}
		res[k - start] = vertex{ C.real(), C.imag() };
	}
	return res;
}
