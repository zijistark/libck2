#include "ProvMap.h"

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <unordered_map>

#include "BMPHeader.h"
#include "Color.h"
#include "DefaultMap.h"
#include "DefinitionsTable.h"
#include "FileLocation.h"
#include "VFS.h"


NAMESPACE_CK2;


ProvMap::ProvMap(const VFS& vfs, const DefaultMap& dm, const DefinitionsTable& def_tbl)
: _map(nullptr),
  _cols(0),
  _rows(0)
{
  /* map provinces.bmp color to province ID */
  std::unordered_map<RGB, prov_id_t> color2id_map;

  for (const auto& row : def_tbl)
    color2id_map.emplace(row.color, row.id);

  const auto path = vfs["map" / dm.province_map_path()];
  const auto spath = path.generic_string();
  const auto ferr = FLErrorStaticFactory(FLoc(path));

  unique_file_ptr ufp( std::fopen(spath.c_str(), "rb"), std::fclose );
  FILE* f = ufp.get();

  if (f == nullptr)
    throw ferr("Failed to open file: {}", strerror(errno));

  BMPHeader bf_hdr;

  if (errno = 0; fread(&bf_hdr, sizeof(bf_hdr), 1, f) < 1)
  {
    if (errno)
      throw ferr("Failed to read bitmap file header: {}", strerror(errno));
    else
      throw ferr("Unexpected EOF while reading bitmap file header (file corruption)");
  }

  if (bf_hdr.magic != BMPHeader::MAGIC)
    throw ferr("Unsupported bitmap file type (magic=0x{:04X} but want magic=0x{:04X})",
           bf_hdr.magic, BMPHeader::MAGIC);

  if (bf_hdr.n_header_size < 40)
    throw ferr("Format unsupported: DIB header size is {} bytes but need at least 40", bf_hdr.n_header_size);

  if (bf_hdr.n_width <= 0)
    throw ferr("Format unsupported: Expected positive image width, found {}", bf_hdr.n_width);

  if (bf_hdr.n_height <= 0)
    throw ferr("Format unsupported: Expected positive image height, found {}", bf_hdr.n_height);

  if (bf_hdr.n_planes != 1)
    throw ferr("Format unsupported: Should only be 1 image plane, found {}", bf_hdr.n_planes);

  if (bf_hdr.n_bpp != 24)
    throw ferr("Format unsupported: Need 24bpp color but found {}", bf_hdr.n_bpp);

  if (bf_hdr.compression_type != 0)
    throw ferr("Format unsupported: Found unsupported compression type #{}", bf_hdr.compression_type);

  if (bf_hdr.n_colors != 0)
    throw ferr("Format unsupported: Image shouldn't be paletted, but {} colors were specified",
           bf_hdr.n_colors);

  assert( bf_hdr.n_important_colors == 0 );

  _cols = bf_hdr.n_width;
  _rows = bf_hdr.n_height;

  /* calculate row size with 32-bit alignment padding */
  uint row_sz = 4 * ((bf_hdr.n_bpp * _cols + 31) / 32);
  const auto bitmap_sz = row_sz * _rows;

  if (bf_hdr.n_bitmap_size != 0 && bf_hdr.n_bitmap_size != bitmap_sz)
    throw ferr("File corruption: Raw bitmap data section should be {} bytes but {} were specified",
           bitmap_sz, bf_hdr.n_bitmap_size);

  /* allocate ID map */
  // OPTIMIZE: align to a 64-byte cache line boundary (or really over-align and choose a page boundary)
  _map = std::make_unique<prov_id_t[]>(_cols * _rows);

  /* seek past any other bytes and directly to offset of pixel array (if needed). */
  if (fseek(f, bf_hdr.n_bitmap_offset, SEEK_SET) != 0)
    throw ferr("Failed to seek to raw bitmap data section (file offset: 0x{0:08X} / {0}): {1}",
           bf_hdr.n_bitmap_offset, strerror(errno));

  /* read bitmap image data (pixel array), row by row, in bottom-to-top raster scan order */
  auto up_row_buf = std::make_unique<uint8_t[]>(row_sz);

  for (uint row = 0; row < _rows; ++row)
  {
    if (errno = 0; fread(up_row_buf.get(), row_sz, 1, f) < 1)
    {
      if (errno)
        throw ferr("Failed to read [bottom-to-top] scanline #{} of bitmap data: {}", row, strerror(errno));
      else
        throw ferr("Unexpected EOF while reading bitmap data");
    }

    const auto y = _rows - 1 - row;

    /* cache previous pixel's value & province ID */
    uint8_t  prev_b = 0;
    uint8_t  prev_g = 0;
    uint8_t  prev_r = 0;
    uint16_t prev_id = 0;

    for (uint x = 0; x < _cols; ++x)
    {
      uint8_t const* p = &up_row_buf[3*x];
      uint16_t id;

      if (p[0] == 0xFF && p[1] == 0xFF && p[2] == 0xFF)
        id = PM_OCEAN;
      else if (p[0] == 0x00 && p[1] == 0x00 && p[2] == 0x00)
        id = PM_IMPASSABLE;
      else if (x > 0 && p[0] == prev_b && p[1] == prev_g && p[2] == prev_r)
        id = prev_id; // save time at the bottleneck of the hash table lookup due to color locality
      else
      {
        if (auto it = color2id_map.find({ p[2], p[1], p[0] }); it != color2id_map.end())
          id = it->second;
        else
          throw ferr("Unexpected color RGB({}, {}, {}) in provinces bitmap at pixel ({}, {})",
                  p[2], p[1], p[0], x, y);
      }

      prev_b = p[0];
      prev_g = p[1];
      prev_r = p[2];
      prev_id = _map[y*_cols + x] = id;
    }
  }
}


NAMESPACE_CK2_END;
