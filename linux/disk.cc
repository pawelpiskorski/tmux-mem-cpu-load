/* vim: tabstop=2 shiftwidth=2 expandtab textwidth=80 linebreak wrap
 *
 * Copyright 2012 Matthew McCormick
 * Copyright 2015 Pawel 'l0ner' Soltys
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <iostream>
#include <sys/statvfs.h>

#include <sstream>
#include <fstream>
#include <sys/sysinfo.h>

#include "memory.h"
#include "conversions.h"
#include "luts.h"
#include "powerline.h"

/**
 * Gets available disk space on the root partition in bytes
 * @return Available space in bytes, or -1 on error
 */
//long long getAvailableDiskSpace() {
void disk_status( MemoryStatus & status ) {
    struct statvfs fs_stats;
    
    // Get filesystem statistics for root partition
    if (statvfs("/", &fs_stats) != 0) 
        throw std::runtime_error("Error getting disk space information");
    
    // Calculate available space in bytes
    long long available_space = static_cast<long long>(fs_stats.f_bavail) *
                               static_cast<long long>(fs_stats.f_frsize);
    long long total =  static_cast<long long>(fs_stats.f_blocks) *
           static_cast<long long>(fs_stats.f_frsize);
    
    status.used_mem = convert_unit(static_cast< float >( available_space ), MEGABYTES, BYTES);
    status.total_mem = convert_unit(static_cast< float >( total ), MEGABYTES, BYTES);
}


std::string disk_string( const MemoryStatus & mem_status,
  MEMORY_MODE mode,
  bool use_colors,
  bool use_powerline_left,
  bool use_powerline_right,
  bool segments_to_left,
  short left_color )
{
  std::ostringstream oss;
  // Change the percision for floats, for a pretty output
  oss.precision( 2 );
  oss.setf( std::ios::fixed | std::ios::right );

  unsigned int color = static_cast< unsigned int >((100 * mem_status.used_mem) / mem_status.total_mem);
  if( use_colors )
  {
    if( use_powerline_right && segments_to_left )
    {
      powerline_char( oss, mem_lut[color], left_color, POWERLINE_RIGHT, false);
      oss << ' ';
    }
    else if( use_powerline_right && !segments_to_left )
    {
      oss << "#[bg=default]";
      powerline( oss, mem_lut[color], POWERLINE_RIGHT );
      oss << ' ';
    }
    else if( use_powerline_left && segments_to_left )
    {
      powerline_char( oss, mem_lut[color], left_color, POWERLINE_LEFT, false);
      oss << ' ';
    }
    else if( use_powerline_left )
    {
      //powerline( oss, mem_lut[color], POWERLINE_LEFT );
      // We do not know how to invert the default background color
      powerline( oss, mem_lut[color], NONE );
      oss << ' ';
    }
    else
    {
      powerline( oss, mem_lut[color], NONE );
    }
  }
  oss << "🖴 ";

  switch( mode )
  {
  case MEMORY_MODE_FREE_MEMORY: // Show free memory in MB or GB
    {
    const float free_mem = mem_status.total_mem - mem_status.used_mem;
    const float free_mem_in_gigabytes = convert_unit( free_mem, GIGABYTES, MEGABYTES );

    // if free memory is less than 1 GB, use MB instead
    if(  free_mem_in_gigabytes < 1.0f )
    {
      oss << free_mem << "MB";
    }
    else
    {
      oss << free_mem_in_gigabytes << "GB";
    }
    break;
    }
  case MEMORY_MODE_USAGE_PERCENTAGE:
    {
    // Calculate the percentage of used memory
    const float percentage_mem = mem_status.used_mem /
      static_cast<float>( mem_status.total_mem ) * 100.0;

    oss << percentage_mem << '%';
    break;
    }
  default: // Default mode, just show the used/total memory in MB
    if(mem_status.used_mem>10000 && mem_status.total_mem>10000)
      oss<<static_cast<unsigned int>(mem_status.used_mem/1024)<<"/"<<static_cast<unsigned int>(mem_status.total_mem/1024)<<"GB";
    else if(mem_status.used_mem<10000 && mem_status.total_mem>10000)
      oss<<static_cast<unsigned int>(mem_status.used_mem)<<"MB/"<<static_cast<unsigned int>(mem_status.total_mem/1024)<<"GB";
    else
      oss<<static_cast<unsigned int>(mem_status.used_mem)<<"/"<<static_cast<unsigned int>(mem_status.total_mem)<<"MB";
  }

  if( use_colors )
  {
    if( use_powerline_left )
    {
      powerline( oss, mem_lut[color], POWERLINE_LEFT, true );
    }
    else if( !use_powerline_right )
    {
      oss << "#[fg=default,bg=default]";
    }
  }

  return oss.str();
}
