#include <utility>
#include <array>
#include <type_traits>
#include <stdexcept>

class PointCoordIndexOutOfRangeError : std::out_of_range
{
public:
    explicit PointCoordIndexOutOfRangeError()
        : std::out_of_range{ "" }
    {}
    const char* what() const noexcept override
    {
        return "Point coordinate index is out of range";
    }
    ~PointCoordIndexOutOfRangeError() noexcept override = default;
};

enum class coords_e : int {
    x = 0,
    y = 1,
    z = 2
};

template <class coordinate_T, int num_dimensions>
class Point {
public:
    static_assert(num_dimensions >= 1, "Must have at least 1 dimension");
    static_assert(std::is_integral<coordinate_T>::value || std::is_floating_point<coordinate_T>::value, "Must be a number");
    static_assert(!std::is_const<coordinate_T>::value, "Point value must not be const");
    static_assert(!std::is_reference<coordinate_T>::value, "Point value must not be a reference");
private:
    std::array<coordinate_T, num_dimensions> m_coordinates;
public:
    // Initialize "new Point{}" to get zero-initialization,
    // Initialize "new Point" to get garbage values.
    constexpr Point() = default;

    template <class... Types>
    explicit constexpr Point(Types&&... coordinates)
        : m_coordinates{ std::forward<Types>(coordinates)... }
    {
        static_assert(int{ sizeof...(coordinates) } == num_dimensions, "Wrong number of initializer list values for Point");
    }
    template <coords_e coord_name>
    constexpr const coordinate_T& at() const&
    {
        return this->at<static_cast<int>(coord_name)>();
    }
    template <coords_e coord_name>
    constexpr coordinate_T& at() &
    {
        return this->at<static_cast<int>(coord_name)>();
    }
    template <int coordinate_index>
    constexpr const coordinate_T& at() const&
    {
        static_assert(coordinate_index >= 0 && coordinate_index < num_dimensions, R"("get" index out of range.)");
        return this->m_coordinates[coordinate_index];
    }
    template <int coordinate_index>
    constexpr coordinate_T& at() &
    {
        static_assert(coordinate_index >= 0 && coordinate_index < num_dimensions, R"("get" index out of range.)");
        return this->m_coordinates[coordinate_index];
    }
    constexpr coordinate_T& at(const int coordinate_index) const&
    {
        if (coordinate_index < 0 || coordinate_index >= num_dimensions) {
            throw PointCoordIndexOutOfRangeError{};
        }
        return this->m_coordinates[coordinate_index];
    }
    constexpr coordinate_T& at(const int coordinate_index) &
    {
        if (coordinate_index < 0 || coordinate_index >= num_dimensions) {
            throw PointCoordIndexOutOfRangeError{};
        }
        return this->m_coordinates[coordinate_index];
    }
    constexpr const coordinate_T& operator[](const int coordinate_index) const&
    {
        return this->m_coordinates[coordinate_index];
    }
    constexpr coordinate_T& operator[](const int coordinate_index) &
    {
        return this->m_coordinates[coordinate_index];
    }
};

template <class FirstArg_T , class... Args_T>
auto make_point(FirstArg_T&& first_arg, Args_T&&... args)
{
    static constexpr int num_dimensions{ sizeof...(args) + 1 };
    using clean_first_typename = typename std::remove_const<typename std::remove_reference<decltype(first_arg)>::type>::type;
    using point_t = Point<clean_first_typename, num_dimensions>;
    return point_t{ first_arg, std::forward<Args_T>(args)... };
}

class List3
{
        std::string m_one{};
        std::string m_two{};
        std::string m_three{};
public:
    List3() = delete;
    List3(std::string one, std::string two, std::string three)
        : m_one{ std::move(one) },
          m_two{ std::move(two) },
          m_three{ std::move(three) }
    {}
    const std::string& one() const& { return this->m_one; }
    const std::string& two() const& { return this->m_two; }
    const std::string& three() const& { return this->m_three; }
};

template <class... ParamPack_T>
List3 make_list_3(ParamPack_T&&... template_parameter_pack)
{
    const int num_elements_in_pack{ sizeof...(template_parameter_pack) };
    static_assert(num_elements_in_pack == 3, "Function requires 3 arguments.");
    return List3(std::forward<ParamPack_T>(template_parameter_pack)...);
}


#include <iostream>

// Base case, no parameters left to unpack
template <int num_elems> void print_all_impl() {}

template <int num_elems, class Elem_T, class... ParamPack_T>
void print_all_impl(const Elem_T& elem, const ParamPack_T&... template_parameter_pack) {
    static constexpr int num_elems_left{ sizeof...(template_parameter_pack) };
    if (num_elems == num_elems_left) {
        std::cout << "{ ";
    }
    std::cout << elem;
    if (num_elems_left == 0) {
        std::cout << " }";
    } else {
        std::cout << ", ";
    }
    print_all_impl<num_elems>(template_parameter_pack...);
}

template <class Elem_T, class... ParamPack_T>
void print_all(const Elem_T& elem, const ParamPack_T&... template_parameter_pack) {
    static constexpr int original_num_elems{ sizeof...(template_parameter_pack) };
    print_all_impl<original_num_elems>(elem, template_parameter_pack...);
}

int main() {
    print_all(1, 2, 3, "hello", 5.0f, 3.14159, 'A');
    std::cout << '\n';
    const auto lst = make_list_3("yes", "", "");
    print_all(lst.one(), lst.two(), lst.three());
    std::cout << '\n';
    make_point(7.8, 9.8f, 3.141596f);
    //make_point(7.8f, 9.8f, 3.141596);
    return 0;
}
