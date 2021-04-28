// Serializer.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <array>
#include <cstddef>
#include <concepts>
#include <functional>

template<class F, class R, class... argv>
concept invocable_r = std::is_invocable_r_v<R, F, argv...>;

class serializer {
private:
	struct warn {
		warn() = delete;
		warn(const warn &) = delete;
		warn(warn &&) = delete;

		static bool pointer_being_serialized;
		static std::function<void()> on_warn;
	};

	struct error {
		error() = delete;
		error(const error &) = delete;
		error(error &&) = delete;

		static bool pointer_being_serialized;
		static std::function<void()> on_error;
	};

	template<class T>
	static constexpr void error_and_warning_check() {
		if constexpr (std::is_pointer_v<T>) {
			// Error overrides Warning, execute Error first

			if (error::pointer_being_serialized) {
				error::on_error();
			} else if (warn::pointer_being_serialized) {
				warn::on_warn();
			}
		}
	}
public:
	template<invocable_r<void> I>
	static void set_on_warn(I action) {
		warn::on_warn = action;
	}

	template<invocable_r<void> I>
	static void set_on_error(I action) {
		error::on_error = action;
	}

	serializer() = delete;
	serializer(const serializer &) = delete;
	serializer(serializer &&) = delete;

	template<class T, std::streamsize OFFSET = 0>
	using bytes = std::array<std::byte, (sizeof(T) * sizeof(std::byte)) + OFFSET>;

	template<class T, std::streamsize OFFSET = 0>
	static constexpr bytes<T, OFFSET> serialize(const T &t) {
		error_and_warning_check<T>();

		bytes<T, OFFSET> data;

		const std::byte *raw_data = reinterpret_cast<const std::byte *> (&t);

		size_t i = 0;

		if constexpr (OFFSET > 0) {
			for (; i < OFFSET; i += sizeof(std::byte)) {
				data[i] = std::byte{ 0 };
			}
		}

		for (; i < sizeof(T); i += sizeof(std::byte)) {
			data[i] = *(raw_data + i);
		}

		return data;
	}

	template<class T, std::streamsize OFFSET = 0>
	static void serialize(const T &t, std::ostream &os) {
		error_and_warning_check<T>();

		if constexpr (OFFSET > 0) {
			os.write(reinterpret_cast<const char *>(0U), OFFSET); // NULL byte padding
		}

		os.write(reinterpret_cast<const char *>(&t), sizeof(T) * sizeof(char));
	}

	template<class T>
	static constexpr void deserialize(bytes<T, 0> data, T &p_t) {
		p_t = *reinterpret_cast<T *>(data.data());
	}

	template<class T, std::streamsize OFFSET = 0>
	static void deserialize(std::istream &is, T &t) {
		if (OFFSET > 0) {
			char offset_temp_holder[OFFSET];

			is.read(offset_temp_holder, OFFSET);
		}

		is.read(reinterpret_cast<char *>(&t), sizeof(T) * sizeof(std::byte));
	}
};

bool serializer::warn::pointer_being_serialized = true;
std::function<void()> serializer::warn::on_warn;

bool serializer::error::pointer_being_serialized = true;
std::function<void()> serializer::error::on_error;

int main() {
	// Testing

	int a = 27;
	int b = 0;

	serializer::bytes<int, 0> bytes = serializer::serialize(a);
	serializer::deserialize(bytes, b);

	std::cout << b << "\n";

	return 0;
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
