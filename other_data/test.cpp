#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

std::vector<float> linspace(float start, float end, int num)
{
    std::vector<float> vec;
    double delta = (end - start) / (num - 1);
    for (int i = 0; i < num - 1; i++)
    {
        vec.push_back(start + delta * i);
    }
    vec.push_back(end);
    return vec;
}

std::vector<float> getF(float start, float end, int n_flat, int n_raise, int n_down)
{
    std::vector<float> vec;
    std::vector<float> raise = linspace(0, end, n_raise);
    std::vector<float> flat = linspace(end, end, n_flat);
    std::vector<float> down = linspace(end, start, n_down * 2);
    std::vector<float> flat2 = linspace(start, start, n_flat);
    std::vector<float> raise2 = linspace(start, 0, n_raise);
    std::copy(raise.begin(), raise.end(), std::back_inserter(vec));
    std::copy(flat.begin(), flat.end(), std::back_inserter(vec));
    std::copy(down.begin(), down.end(), std::back_inserter(vec));
    std::copy(flat2.begin(), flat2.end(), std::back_inserter(vec));
    std::copy(raise2.begin(), raise2.end(), std::back_inserter(vec));
    return vec;
}
int main()
{
    std::vector<float> vec = getF(-10,10,10,10,10);
    for (float num: vec)
    {
        std::cout << num << " ";
    }
    return 0;

}