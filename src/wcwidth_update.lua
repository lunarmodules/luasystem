#!/usr/bin/env lua

-- This file downloads and parses unicode standard files and updates the wcwidth code
-- based on that data.

local VERSION="17.0.0"   -- the unicode standard version to download



-- test if curl is available, and Penlight
do
  local ok, ec = os.execute("curl --version > /dev/null 2>&1")
  if not ok then
    error("curl is not available in the path; exitcode " .. ec)
  end

  local ok, utils = pcall(require, "pl.utils")
  if not ok then
    error("Penlight is not available, please install via `luarocks install penlight`")
  end

  utils.readfile("./wcwidth.c")
  if not ok then
    error("failed to read './wcwidth.c', run this script from within the `./src/` directory")
  end
end

-- files to download from the unicode site
local FN_DERIVED_GENERAL_CATEGORY = 1
local FN_EAST_ASIAN_WIDTH = 2
local FN_DERIVED_CORE_PROPERTIES = 3
local FN_EMOJI_DATA = 4

local download_file_list = {
  [FN_DERIVED_GENERAL_CATEGORY] = "extracted/DerivedGeneralCategory.txt",
  [FN_EAST_ASIAN_WIDTH]         = "EastAsianWidth.txt",
  [FN_DERIVED_CORE_PROPERTIES]  = "DerivedCoreProperties.txt",
  [FN_EMOJI_DATA]               = "emoji/emoji-data.txt",
}
local target_path = "./unicode_data/"



do
  local base_url = "https://www.unicode.org/Public/" .. VERSION .. "/ucd/"  -- must include trailing slash


  -- removes a file, and then downloads a new copy from the unicode site
  local function download_file(filename, target_filename)
    print("Downloading " .. filename .. " to " .. target_filename)
    os.remove(target_filename)
    local cmd = "curl --fail -s -o " .. target_filename .. " " .. base_url .. filename
    local ok, ec = os.execute(cmd)
    if not ok then
      error("Failed to execute: " .. cmd .. "; exitcode " .. ec)
    end
  end


  -- Downloads all unicode files we need
  local function download_files()
    os.execute("mkdir -p " .. target_path .. "extracted")
    os.execute("mkdir -p " .. target_path .. "emoji")
    for _, filename in ipairs(download_file_list) do
      download_file(filename, target_path .. filename)
    end
  end


  download_files()
end



-- set up the 3 lists of data (everything else is single-width)
local zero_width = {}
local double_width = {}
local ambiguous_width = {}



local readlines do
  local utils = require("pl.utils")

  function readlines(filename)
    print("Parsing " .. filename)
    local lines = assert(utils.readlines(filename))

    -- drop lines starting with "#" being comments, or empty lines (whitespace only)
    for i = #lines, 1, -1 do -- reverse, since we're deleting items
      if lines[i]:match("^%s*#") or lines[i]:match("^%s*$") then
        table.remove(lines, i)
      end
    end

    return lines
  end
end




-- parse DerivedGeneralCategory.txt
-- Purpose: zero-width combining marks
-- Extract:
--   Mn — Nonspacing Mark → width = 0
--   Me — Enclosing Mark → width = 0
-- Why:
--   These characters overlay the previous glyph
--   This replaces Markus Kuhn’s combining[] table
-- Ignore all other categories in this file.
do
  local lines = readlines(target_path .. download_file_list[FN_DERIVED_GENERAL_CATEGORY])
  local zw_start = #zero_width

  -- parse the lines
  for _, line in ipairs(lines) do
    local range, category = line:match("^([%x%.]+)%s*;%s*(%a+)")
    if not range then
      error("Failed to parse line: " .. line)
    end

    if not range:find("..", 1, true) then -- single code point, make range
      range = range .. ".." .. range
    end

    if category == "Mn" or category == "Me" then
      zero_width[#zero_width + 1] = range
    end
  end

  print("  found " .. (#zero_width - zw_start) .. " zero-width character-ranges")
end



-- parse DerivedCoreProperties.txt
-- Purpose: zero-width format / ignorable characters
-- Extract:
--   Default_Ignorable_Code_Point → width = 0

-- Includes (important examples):
--   U+200D ZERO WIDTH JOINER
--   U+200C ZERO WIDTH NON-JOINER
--   U+FE00..U+FE0F (variation selectors)
--   Bidi and other format controls

-- Why:
--   Not Mn/Me, but terminals treat them as zero-width
--   Required for emoji correctness and modern text
do
  local lines = readlines(target_path .. download_file_list[FN_DERIVED_CORE_PROPERTIES])
  local zw_start = #zero_width

  -- parse the lines
  for _, line in ipairs(lines) do
    local range, category = line:match("^([%x%.]+)%s*;%s*([%a_]+)")
    if not range then
      error("Failed to parse line: " .. line)
    end

    if not range:find("..", 1, true) then -- single code point, make range
      range = range .. ".." .. range
    end

    if category == "Default_Ignorable_Code_Point" then
      zero_width[#zero_width + 1] = range
    end
  end

  print("  found " .. (#zero_width - zw_start) .. " zero-width character-ranges")
end



-- parse EastAsianWidth.txt
-- Purpose: determine double-width and ambiguous-width characters
-- Extract:
--   W (Wide) → width = 2
--   F (Fullwidth) → width = 2
--   A (Ambiguous) → width = 1 or 2 (your choice; usually 1 unless CJK mode)
-- Everything else:
--   H, Na, N → width = 1
-- Why:
--   - This is the only Unicode-sanctioned width-related property
--   - Core of all wcwidth() implementations
do
  local lines = readlines(target_path .. download_file_list[FN_EAST_ASIAN_WIDTH])
  local dw_start = #double_width
  local aw_start = #ambiguous_width

  -- parse the lines
  for _, line in ipairs(lines) do
    local range, width_type = line:match("^([%x%.]+)%s*;%s*(%a+)")
    if not range then
      error("Failed to parse line: " .. line)
    end

    if not range:find("..", 1, true) then -- single code point, make range
      range = range .. ".." .. range
    end

    if width_type == "W" or width_type == "F" then
      double_width[#double_width + 1] = range
    elseif width_type == "A" then
      ambiguous_width[#ambiguous_width + 1] = range
    end
  end

  print("  found " .. (#double_width - dw_start) .. " double-width character-ranges")
  print("  found " .. (#ambiguous_width - aw_start) .. " ambiguous-width character-ranges")
end



-- parse emoji-data.txt
-- Purpose: emoji presentation width
-- Extract:
--   Emoji_Presentation=Yes → width = 2
--   (Optionally) Extended_Pictographic → emoji sequences
-- Why:
--   Emoji are not reliably covered by EastAsianWidth
--   Modern terminals render these as double-width
--   Required for correct emoji column alignment
do
  local lines = readlines(target_path .. download_file_list[FN_EMOJI_DATA])
  local dw_start = #double_width

  -- parse the lines
  for _, line in ipairs(lines) do
    local range, properties = line:match("^([%x%.]+)%s*;%s*([%a_]+)")
    if not range then
      error("Failed to parse line: " .. line)
    end

    if not range:find("..", 1, true) then -- single code point, make range
      range = range .. ".." .. range
    end

    if properties:match("Emoji_Presentation") then
      double_width[#double_width + 1] = range
    end
  end

  print("  found " .. (#double_width - dw_start) .. " double-width character-ranges")
end



-- returns the start and end of a range, numerically, and hex strings
-- @tparam string range the range to parse
-- @treturn number sr the start of the range
-- @treturn number er the end of the range
-- @treturn string sh the start of the range as a hex string
-- @treturn string eh the end of the range as a hex string
local parse_range do
  function parse_range(range)
    local s = range:find("..", 1, true)
    if not s then
      error("Failed to parse range: " .. range)
    end
    local sh = range:sub(1, s - 1)
    local eh = range:sub(s + 2, -1)
    local sr = tonumber(sh, 16)
    local er = tonumber(eh, 16)
    if er < sr then
      error("Failed to parse range: " .. range .. " (end < start)")
    end
    return sr, er, sh, eh
  end

  -- some inline tests for parse_range
  local sr, er = parse_range("25FD..25FE")
  assert(sr == 9725)
  assert(er == 9726)
  local sr, er = parse_range("105C0..105F3")
  assert(sr == 67008)
  assert(er == 67059)
end



-- sorts the ranges in-place
local function sort_ranges(ranges)
  table.sort(ranges, function(a, b)
    return parse_range(a) < parse_range(b)
  end)
  return ranges
end



-- combines adjacent ranges in-place
local combine_ranges do
  function combine_ranges(ranges)
    local last_idx = 1
    for i = 2, #ranges do
      local last_s, last_e, last_sh, last_eh = parse_range(ranges[last_idx])
      local current_s, current_e, _, current_eh = parse_range(ranges[i])
      if current_s >= last_s and current_s <= (last_e + 1) then
        -- ranges are adjacent or overlapping, combine them
        local sh = last_sh
        local eh = current_eh
        if last_e > current_e then
          eh = last_eh
        end
        ranges[last_idx] = sh .. ".." .. eh
      else
        last_idx = last_idx + 1
        ranges[last_idx] = ranges[i]
      end
    end
    -- clear left-overs beyond last entry
    for i = last_idx + 1, #ranges do
      ranges[i] = nil
    end
  end

  -- some inline tests for combine_ranges
  local ranges = {
    "25FD..25FE",
    "25FD..25FE",  -- duplicate range, should be removed
    "105C0..105F3",
    "105D0..105E0",  -- range fully within previous range, should be combined
    "10F00..10F10",
    "10F11..10F20",  -- adjacent or previous, should be combined
    "11000..11100",
    "11101..11110",  -- adjacent + extending to previous, should be combined
    "12000..12010",
    "12011..12020",  -- multiple: adjacent should be combined
    "12015..12030",  -- multiple: overlap + extending to previous, should be combined
    "12031..12040",  -- multiple: overlapping, should be combined
  }
  combine_ranges(ranges)
  assert(#ranges == 5)
  assert(ranges[1] == "25FD..25FE")
  assert(ranges[2] == "105C0..105F3")
  assert(ranges[3] == "10F00..10F20")
  assert(ranges[4] == "11000..11110")
  assert(ranges[5] == "12000..12040")
end



combine_ranges(sort_ranges(zero_width))
combine_ranges(sort_ranges(double_width))
combine_ranges(sort_ranges(ambiguous_width))



-- convert ranges into c-source-code ranges (in-place)
-- format: "{ 0x0829, 0x082D }"
local function convert_c_ranges(ranges)
  for i = 1, #ranges do
    local _, _, sh, eh = parse_range(ranges[i])
    ranges[i] = "{ 0x" .. sh .. ", 0x" .. eh .. " }"
  end
end

convert_c_ranges(zero_width)
convert_c_ranges(double_width)
convert_c_ranges(ambiguous_width)



local SOURCE_INDENT = "    "


-- write c source, as triplet; 3 ranges on 1 line
local function triplet_lines(ranges)
  local lines = {}
  for i = 1, #ranges, 3 do
    lines[#lines+1] = SOURCE_INDENT .. table.concat(ranges, ", ", i, math.min(i + 2, #ranges)) .. ","
  end
  -- drop trailing comma from last line
  lines[#lines] = lines[#lines]:sub(1, -2)
  return lines
end


-- create file-contents
local function create_file_contents(ranges, contains)
  return
    SOURCE_INDENT .. "// Do not modify this file directly, it is generated by the wcwidth_update.lua script\n" ..
    SOURCE_INDENT .. "// Contains " .. contains .. "\n" ..
    SOURCE_INDENT .. "// Generated from Unicode " .. VERSION .. "\n" ..
    SOURCE_INDENT .. "// Generated on " .. os.date("%Y-%m-%d") .. "\n" ..
    table.concat(triplet_lines(ranges), "\n") .. "\n"
end




local writefile = require("pl.utils").writefile

print("writing source files...")
print("  zero-width: ./wcwidth_zero_width.c")
assert(writefile("./wcwidth_zero_width.c", create_file_contents(zero_width, "unicode character-ranges handled as 0 width")))

print("  double-width: ./wcwidth_double_width.c")
assert(writefile("./wcwidth_double_width.c", create_file_contents(double_width, "unicode character-ranges handled as double width")))

print("  ambiguous-width: ./wcwidth_ambiguous_width.c")
assert(writefile("./wcwidth_ambiguous_width.c", create_file_contents(ambiguous_width, "unicode character-ranges handled as ambiguous (either 1 or 2 width)")))
