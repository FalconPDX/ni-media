//
// Copyright (c) 2017 Native Instruments GmbH, Berlin
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//

#pragma once

#include <ni/media/audio/ifstream_info.h>
#include <ni/media/audio/ofstream_info.h>

#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

#include <map>
#include <set>

namespace audio
{

auto ifstream_container( const boost::filesystem::path& path ) -> boost::optional<ifstream_info::container_type>;
auto ofstream_container( const boost::filesystem::path& path ) -> boost::optional<ofstream_info::container_type>;

bool can_read_file( const boost::filesystem::path& path );
bool can_read_file( const boost::filesystem::path& path, std::set<ifstream_info::container_type> supported_containers );

bool can_write_file( const boost::filesystem::path& path );
bool can_write_file( const boost::filesystem::path&          path,
                     std::set<ofstream_info::container_type> supported_containers );
}