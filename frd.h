// utilities for dealing with FRD loudspeaker measurements in text files

// Copyright 2019 Michael Mayne

// Permission is hereby granted, free of charge, to any person obtaining a copy of this 
// software and associated documentation files (the "Software"), to deal in the Software without restriction, 
// including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
// and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
// subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO 
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE 
// OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <istream>
#include <vector>
#include <sstream>
#include <stdexcept>    // std::invalid_argument
#include <algorithm>    // std::lower_bound


#if __has_include("experimental/filesystem")
    #include <filesystem> // gcc 8, untested 
    namespace filesystem = std::filesystem; // gcc 8
#elif __has_include("experimental/filesystem")
    #include <experimental/filesystem> // gcc 7
    namespace filesystem = std::experimental::filesystem; // gcc 7
#else
    #include <boost/filesystem.hpp>
    namespace filesystem = boost::filesystem;
#endif

namespace frd {

// A single data point from an FRD file
template <typename FloatType>
struct FRDValue
{
    FloatType freqHz;   // w?
    FloatType dBSPL;    // real Pa?  see what works 
    FloatType phaseDeg; // imag Pa?  with the calculator
};

template <typename FloatType>
std::string str(const FRDValue<FloatType>& value)
{
    std::ostringstream oss;
    oss << value.freqHz << "Hz " << value.dBSPL << "dB " << value.phaseDeg << "deg";
    return oss.str();
}

// parses data that looks like this:
// Freq [Hz]       dBSPL           Phase [Deg]
// 10.00           75.18           83.73
//
// and returns a std::vector of FRDValues
// treats any line that starts with a non-numeric character as a comment
// throws std::invalid_argument if any of the data lines cannot be parsed
template<typename FloatType>
std::vector<FRDValue<FloatType>> parse(std::istream& text)
{
    std::vector<FRDValue<FloatType>> frd;
    std::string buffer;
    while (std::getline(text, buffer))
    {
        if (isalpha(buffer[0])) // comment line
            continue;
        
        std::stringstream ss(buffer);
        FRDValue<FloatType> frdValue;

        if (!(ss >> frdValue.freqHz >> frdValue.dBSPL >> frdValue.phaseDeg))
        {
            throw std::invalid_argument("Invalid input in line: " + buffer);
        }
        frd.push_back(frdValue);
    }
    return frd;
}

template<typename FloatType>
struct less_than_freq
{
    inline bool operator() (const FRDValue<FloatType>& frd1, const FRDValue<FloatType>& frd2)
    {
        return (frd1.freqHz < frd2.freqHz);
    }
};

// Takes a vector of FRDValues sorted by Frequency and a frequency 
//
// returns the an iterator to the FRDValue 
// where FRDValue.freqHz compares less than freq.
//
// If findFreq(frd, freq) == frd.end() the frequency was not found
template<typename FloatType>
typename std::vector<FRDValue<FloatType>>::iterator
findFreq(std::vector<FRDValue<FloatType>>& Frd, FloatType freq)
{   
    FRDValue<FloatType> freqHelper {freq, static_cast<FloatType>(0), static_cast<FloatType>(0)};
    return std::lower_bound(Frd.begin(), Frd.end(), freqHelper, less_than_freq<FloatType>());
}

// PolarData holds the dB spl, phase data for all frequencies
// Its not used in the critical path
template<typename FloatType>
using PolarData = std::vector<std::vector<FRDValue<FloatType>>>;

// Imports each file as a vector of FRDValues
// Will import them in alphabetical order, make sure the files are named correctly
template<typename FloatType>
PolarData<FloatType> import_polardata(std::string folder)
{
    auto it = filesystem::directory_iterator(folder);
    std::vector<filesystem::directory_entry> files;
    std::copy(filesystem::begin(it), filesystem::end(it), std::back_inserter(files));
    std::sort(files.begin(), files.end());

    PolarData<FloatType> p;

    for (const auto & file : files)
    {
        std::ifstream fs(file.path().c_str());
        p.push_back(parse<FloatType>(fs));
    }

    return p;
}

}; // frd
