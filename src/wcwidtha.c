// To update this file to the lastest version of the Unicode standard
// save the Lua script below to a file named 'getranges.lua'
// execute as:
// curl -s https://www.unicode.org/Public/UCD/latest/ucd/EastAsianWidth.txt | lua getranges.lua

/* the script:
local function singleline(line)
  if line:find("; A", 1, true) then -- handle ambiguous characters only
    local s,e = line:match("^([0-9a-fA-F]+)%.?%.?([0-9a-fA-F]*)")
    e = (e == "" and s) or e -- single char, so end-range == start-range
    local cmmnt = "// "..line:match("(; A.*)$")
    local range = "        {0x"..s..", 0x"..e.."},"
    print(range..(" "):rep(30-#range)..cmmnt) -- print formatted output line
  end
end

-- read all lines from stdin and iterate over them
local t = {}
for line in io.lines() do
  line = line:match("^%s*(.-)%s*$") -- strip whitespace
  if line ~= "" and line:sub(1,1) ~= "#" then -- skip comments and empty lines
    singleline(line)
  end
end
*/


#include "wcwidtha.h"

struct interval {
    mk_wchar_t start;
    mk_wchar_t end;
};


// Takes a unicode character, and return whether the character is in the list of
// ambiguous width characters.
int mk_wcwidth_a(mk_wchar_t ucs)
{
    /* sorted list of ambiguous width characters in East Asian displays */
    /* generated by script in the comments above */
    static const struct interval ranges[] = {
        {0x00A1, 0x00A1},     // ; A  # Po         INVERTED EXCLAMATION MARK
        {0x00A4, 0x00A4},     // ; A  # Sc         CURRENCY SIGN
        {0x00A7, 0x00A7},     // ; A  # Po         SECTION SIGN
        {0x00A8, 0x00A8},     // ; A  # Sk         DIAERESIS
        {0x00AA, 0x00AA},     // ; A  # Lo         FEMININE ORDINAL INDICATOR
        {0x00AD, 0x00AD},     // ; A  # Cf         SOFT HYPHEN
        {0x00AE, 0x00AE},     // ; A  # So         REGISTERED SIGN
        {0x00B0, 0x00B0},     // ; A  # So         DEGREE SIGN
        {0x00B1, 0x00B1},     // ; A  # Sm         PLUS-MINUS SIGN
        {0x00B2, 0x00B3},     // ; A  # No     [2] SUPERSCRIPT TWO..SUPERSCRIPT THREE
        {0x00B4, 0x00B4},     // ; A  # Sk         ACUTE ACCENT
        {0x00B6, 0x00B7},     // ; A  # Po     [2] PILCROW SIGN..MIDDLE DOT
        {0x00B8, 0x00B8},     // ; A  # Sk         CEDILLA
        {0x00B9, 0x00B9},     // ; A  # No         SUPERSCRIPT ONE
        {0x00BA, 0x00BA},     // ; A  # Lo         MASCULINE ORDINAL INDICATOR
        {0x00BC, 0x00BE},     // ; A  # No     [3] VULGAR FRACTION ONE QUARTER..VULGAR FRACTION THREE QUARTERS
        {0x00BF, 0x00BF},     // ; A  # Po         INVERTED QUESTION MARK
        {0x00C6, 0x00C6},     // ; A  # Lu         LATIN CAPITAL LETTER AE
        {0x00D0, 0x00D0},     // ; A  # Lu         LATIN CAPITAL LETTER ETH
        {0x00D7, 0x00D7},     // ; A  # Sm         MULTIPLICATION SIGN
        {0x00D8, 0x00D8},     // ; A  # Lu         LATIN CAPITAL LETTER O WITH STROKE
        {0x00DE, 0x00E1},     // ; A  # L&     [4] LATIN CAPITAL LETTER THORN..LATIN SMALL LETTER A WITH ACUTE
        {0x00E6, 0x00E6},     // ; A  # Ll         LATIN SMALL LETTER AE
        {0x00E8, 0x00EA},     // ; A  # Ll     [3] LATIN SMALL LETTER E WITH GRAVE..LATIN SMALL LETTER E WITH CIRCUMFLEX
        {0x00EC, 0x00ED},     // ; A  # Ll     [2] LATIN SMALL LETTER I WITH GRAVE..LATIN SMALL LETTER I WITH ACUTE
        {0x00F0, 0x00F0},     // ; A  # Ll         LATIN SMALL LETTER ETH
        {0x00F2, 0x00F3},     // ; A  # Ll     [2] LATIN SMALL LETTER O WITH GRAVE..LATIN SMALL LETTER O WITH ACUTE
        {0x00F7, 0x00F7},     // ; A  # Sm         DIVISION SIGN
        {0x00F8, 0x00FA},     // ; A  # Ll     [3] LATIN SMALL LETTER O WITH STROKE..LATIN SMALL LETTER U WITH ACUTE
        {0x00FC, 0x00FC},     // ; A  # Ll         LATIN SMALL LETTER U WITH DIAERESIS
        {0x00FE, 0x00FE},     // ; A  # Ll         LATIN SMALL LETTER THORN
        {0x0101, 0x0101},     // ; A  # Ll         LATIN SMALL LETTER A WITH MACRON
        {0x0111, 0x0111},     // ; A  # Ll         LATIN SMALL LETTER D WITH STROKE
        {0x0113, 0x0113},     // ; A  # Ll         LATIN SMALL LETTER E WITH MACRON
        {0x011B, 0x011B},     // ; A  # Ll         LATIN SMALL LETTER E WITH CARON
        {0x0126, 0x0127},     // ; A  # L&     [2] LATIN CAPITAL LETTER H WITH STROKE..LATIN SMALL LETTER H WITH STROKE
        {0x012B, 0x012B},     // ; A  # Ll         LATIN SMALL LETTER I WITH MACRON
        {0x0131, 0x0133},     // ; A  # L&     [3] LATIN SMALL LETTER DOTLESS I..LATIN SMALL LIGATURE IJ
        {0x0138, 0x0138},     // ; A  # Ll         LATIN SMALL LETTER KRA
        {0x013F, 0x0142},     // ; A  # L&     [4] LATIN CAPITAL LETTER L WITH MIDDLE DOT..LATIN SMALL LETTER L WITH STROKE
        {0x0144, 0x0144},     // ; A  # Ll         LATIN SMALL LETTER N WITH ACUTE
        {0x0148, 0x014B},     // ; A  # L&     [4] LATIN SMALL LETTER N WITH CARON..LATIN SMALL LETTER ENG
        {0x014D, 0x014D},     // ; A  # Ll         LATIN SMALL LETTER O WITH MACRON
        {0x0152, 0x0153},     // ; A  # L&     [2] LATIN CAPITAL LIGATURE OE..LATIN SMALL LIGATURE OE
        {0x0166, 0x0167},     // ; A  # L&     [2] LATIN CAPITAL LETTER T WITH STROKE..LATIN SMALL LETTER T WITH STROKE
        {0x016B, 0x016B},     // ; A  # Ll         LATIN SMALL LETTER U WITH MACRON
        {0x01CE, 0x01CE},     // ; A  # Ll         LATIN SMALL LETTER A WITH CARON
        {0x01D0, 0x01D0},     // ; A  # Ll         LATIN SMALL LETTER I WITH CARON
        {0x01D2, 0x01D2},     // ; A  # Ll         LATIN SMALL LETTER O WITH CARON
        {0x01D4, 0x01D4},     // ; A  # Ll         LATIN SMALL LETTER U WITH CARON
        {0x01D6, 0x01D6},     // ; A  # Ll         LATIN SMALL LETTER U WITH DIAERESIS AND MACRON
        {0x01D8, 0x01D8},     // ; A  # Ll         LATIN SMALL LETTER U WITH DIAERESIS AND ACUTE
        {0x01DA, 0x01DA},     // ; A  # Ll         LATIN SMALL LETTER U WITH DIAERESIS AND CARON
        {0x01DC, 0x01DC},     // ; A  # Ll         LATIN SMALL LETTER U WITH DIAERESIS AND GRAVE
        {0x0251, 0x0251},     // ; A  # Ll         LATIN SMALL LETTER ALPHA
        {0x0261, 0x0261},     // ; A  # Ll         LATIN SMALL LETTER SCRIPT G
        {0x02C4, 0x02C4},     // ; A  # Sk         MODIFIER LETTER UP ARROWHEAD
        {0x02C7, 0x02C7},     // ; A  # Lm         CARON
        {0x02C9, 0x02CB},     // ; A  # Lm     [3] MODIFIER LETTER MACRON..MODIFIER LETTER GRAVE ACCENT
        {0x02CD, 0x02CD},     // ; A  # Lm         MODIFIER LETTER LOW MACRON
        {0x02D0, 0x02D0},     // ; A  # Lm         MODIFIER LETTER TRIANGULAR COLON
        {0x02D8, 0x02DB},     // ; A  # Sk     [4] BREVE..OGONEK
        {0x02DD, 0x02DD},     // ; A  # Sk         DOUBLE ACUTE ACCENT
        {0x02DF, 0x02DF},     // ; A  # Sk         MODIFIER LETTER CROSS ACCENT
        {0x0300, 0x036F},     // ; A  # Mn   [112] COMBINING GRAVE ACCENT..COMBINING LATIN SMALL LETTER X
        {0x0391, 0x03A1},     // ; A  # Lu    [17] GREEK CAPITAL LETTER ALPHA..GREEK CAPITAL LETTER RHO
        {0x03A3, 0x03A9},     // ; A  # Lu     [7] GREEK CAPITAL LETTER SIGMA..GREEK CAPITAL LETTER OMEGA
        {0x03B1, 0x03C1},     // ; A  # Ll    [17] GREEK SMALL LETTER ALPHA..GREEK SMALL LETTER RHO
        {0x03C3, 0x03C9},     // ; A  # Ll     [7] GREEK SMALL LETTER SIGMA..GREEK SMALL LETTER OMEGA
        {0x0401, 0x0401},     // ; A  # Lu         CYRILLIC CAPITAL LETTER IO
        {0x0410, 0x044F},     // ; A  # L&    [64] CYRILLIC CAPITAL LETTER A..CYRILLIC SMALL LETTER YA
        {0x0451, 0x0451},     // ; A  # Ll         CYRILLIC SMALL LETTER IO
        {0x2010, 0x2010},     // ; A  # Pd         HYPHEN
        {0x2013, 0x2015},     // ; A  # Pd     [3] EN DASH..HORIZONTAL BAR
        {0x2016, 0x2016},     // ; A  # Po         DOUBLE VERTICAL LINE
        {0x2018, 0x2018},     // ; A  # Pi         LEFT SINGLE QUOTATION MARK
        {0x2019, 0x2019},     // ; A  # Pf         RIGHT SINGLE QUOTATION MARK
        {0x201C, 0x201C},     // ; A  # Pi         LEFT DOUBLE QUOTATION MARK
        {0x201D, 0x201D},     // ; A  # Pf         RIGHT DOUBLE QUOTATION MARK
        {0x2020, 0x2022},     // ; A  # Po     [3] DAGGER..BULLET
        {0x2024, 0x2027},     // ; A  # Po     [4] ONE DOT LEADER..HYPHENATION POINT
        {0x2030, 0x2030},     // ; A  # Po         PER MILLE SIGN
        {0x2032, 0x2033},     // ; A  # Po     [2] PRIME..DOUBLE PRIME
        {0x2035, 0x2035},     // ; A  # Po         REVERSED PRIME
        {0x203B, 0x203B},     // ; A  # Po         REFERENCE MARK
        {0x203E, 0x203E},     // ; A  # Po         OVERLINE
        {0x2074, 0x2074},     // ; A  # No         SUPERSCRIPT FOUR
        {0x207F, 0x207F},     // ; A  # Lm         SUPERSCRIPT LATIN SMALL LETTER N
        {0x2081, 0x2084},     // ; A  # No     [4] SUBSCRIPT ONE..SUBSCRIPT FOUR
        {0x20AC, 0x20AC},     // ; A  # Sc         EURO SIGN
        {0x2103, 0x2103},     // ; A  # So         DEGREE CELSIUS
        {0x2105, 0x2105},     // ; A  # So         CARE OF
        {0x2109, 0x2109},     // ; A  # So         DEGREE FAHRENHEIT
        {0x2113, 0x2113},     // ; A  # Ll         SCRIPT SMALL L
        {0x2116, 0x2116},     // ; A  # So         NUMERO SIGN
        {0x2121, 0x2122},     // ; A  # So     [2] TELEPHONE SIGN..TRADE MARK SIGN
        {0x2126, 0x2126},     // ; A  # Lu         OHM SIGN
        {0x212B, 0x212B},     // ; A  # Lu         ANGSTROM SIGN
        {0x2153, 0x2154},     // ; A  # No     [2] VULGAR FRACTION ONE THIRD..VULGAR FRACTION TWO THIRDS
        {0x215B, 0x215E},     // ; A  # No     [4] VULGAR FRACTION ONE EIGHTH..VULGAR FRACTION SEVEN EIGHTHS
        {0x2160, 0x216B},     // ; A  # Nl    [12] ROMAN NUMERAL ONE..ROMAN NUMERAL TWELVE
        {0x2170, 0x2179},     // ; A  # Nl    [10] SMALL ROMAN NUMERAL ONE..SMALL ROMAN NUMERAL TEN
        {0x2189, 0x2189},     // ; A  # No         VULGAR FRACTION ZERO THIRDS
        {0x2190, 0x2194},     // ; A  # Sm     [5] LEFTWARDS ARROW..LEFT RIGHT ARROW
        {0x2195, 0x2199},     // ; A  # So     [5] UP DOWN ARROW..SOUTH WEST ARROW
        {0x21B8, 0x21B9},     // ; A  # So     [2] NORTH WEST ARROW TO LONG BAR..LEFTWARDS ARROW TO BAR OVER RIGHTWARDS ARROW TO BAR
        {0x21D2, 0x21D2},     // ; A  # Sm         RIGHTWARDS DOUBLE ARROW
        {0x21D4, 0x21D4},     // ; A  # Sm         LEFT RIGHT DOUBLE ARROW
        {0x21E7, 0x21E7},     // ; A  # So         UPWARDS WHITE ARROW
        {0x2200, 0x2200},     // ; A  # Sm         FOR ALL
        {0x2202, 0x2203},     // ; A  # Sm     [2] PARTIAL DIFFERENTIAL..THERE EXISTS
        {0x2207, 0x2208},     // ; A  # Sm     [2] NABLA..ELEMENT OF
        {0x220B, 0x220B},     // ; A  # Sm         CONTAINS AS MEMBER
        {0x220F, 0x220F},     // ; A  # Sm         N-ARY PRODUCT
        {0x2211, 0x2211},     // ; A  # Sm         N-ARY SUMMATION
        {0x2215, 0x2215},     // ; A  # Sm         DIVISION SLASH
        {0x221A, 0x221A},     // ; A  # Sm         SQUARE ROOT
        {0x221D, 0x2220},     // ; A  # Sm     [4] PROPORTIONAL TO..ANGLE
        {0x2223, 0x2223},     // ; A  # Sm         DIVIDES
        {0x2225, 0x2225},     // ; A  # Sm         PARALLEL TO
        {0x2227, 0x222C},     // ; A  # Sm     [6] LOGICAL AND..DOUBLE INTEGRAL
        {0x222E, 0x222E},     // ; A  # Sm         CONTOUR INTEGRAL
        {0x2234, 0x2237},     // ; A  # Sm     [4] THEREFORE..PROPORTION
        {0x223C, 0x223D},     // ; A  # Sm     [2] TILDE OPERATOR..REVERSED TILDE
        {0x2248, 0x2248},     // ; A  # Sm         ALMOST EQUAL TO
        {0x224C, 0x224C},     // ; A  # Sm         ALL EQUAL TO
        {0x2252, 0x2252},     // ; A  # Sm         APPROXIMATELY EQUAL TO OR THE IMAGE OF
        {0x2260, 0x2261},     // ; A  # Sm     [2] NOT EQUAL TO..IDENTICAL TO
        {0x2264, 0x2267},     // ; A  # Sm     [4] LESS-THAN OR EQUAL TO..GREATER-THAN OVER EQUAL TO
        {0x226A, 0x226B},     // ; A  # Sm     [2] MUCH LESS-THAN..MUCH GREATER-THAN
        {0x226E, 0x226F},     // ; A  # Sm     [2] NOT LESS-THAN..NOT GREATER-THAN
        {0x2282, 0x2283},     // ; A  # Sm     [2] SUBSET OF..SUPERSET OF
        {0x2286, 0x2287},     // ; A  # Sm     [2] SUBSET OF OR EQUAL TO..SUPERSET OF OR EQUAL TO
        {0x2295, 0x2295},     // ; A  # Sm         CIRCLED PLUS
        {0x2299, 0x2299},     // ; A  # Sm         CIRCLED DOT OPERATOR
        {0x22A5, 0x22A5},     // ; A  # Sm         UP TACK
        {0x22BF, 0x22BF},     // ; A  # Sm         RIGHT TRIANGLE
        {0x2312, 0x2312},     // ; A  # So         ARC
        {0x2460, 0x249B},     // ; A  # No    [60] CIRCLED DIGIT ONE..NUMBER TWENTY FULL STOP
        {0x249C, 0x24E9},     // ; A  # So    [78] PARENTHESIZED LATIN SMALL LETTER A..CIRCLED LATIN SMALL LETTER Z
        {0x24EB, 0x24FF},     // ; A  # No    [21] NEGATIVE CIRCLED NUMBER ELEVEN..NEGATIVE CIRCLED DIGIT ZERO
        {0x2500, 0x254B},     // ; A  # So    [76] BOX DRAWINGS LIGHT HORIZONTAL..BOX DRAWINGS HEAVY VERTICAL AND HORIZONTAL
        {0x2550, 0x2573},     // ; A  # So    [36] BOX DRAWINGS DOUBLE HORIZONTAL..BOX DRAWINGS LIGHT DIAGONAL CROSS
        {0x2580, 0x258F},     // ; A  # So    [16] UPPER HALF BLOCK..LEFT ONE EIGHTH BLOCK
        {0x2592, 0x2595},     // ; A  # So     [4] MEDIUM SHADE..RIGHT ONE EIGHTH BLOCK
        {0x25A0, 0x25A1},     // ; A  # So     [2] BLACK SQUARE..WHITE SQUARE
        {0x25A3, 0x25A9},     // ; A  # So     [7] WHITE SQUARE CONTAINING BLACK SMALL SQUARE..SQUARE WITH DIAGONAL CROSSHATCH FILL
        {0x25B2, 0x25B3},     // ; A  # So     [2] BLACK UP-POINTING TRIANGLE..WHITE UP-POINTING TRIANGLE
        {0x25B6, 0x25B6},     // ; A  # So         BLACK RIGHT-POINTING TRIANGLE
        {0x25B7, 0x25B7},     // ; A  # Sm         WHITE RIGHT-POINTING TRIANGLE
        {0x25BC, 0x25BD},     // ; A  # So     [2] BLACK DOWN-POINTING TRIANGLE..WHITE DOWN-POINTING TRIANGLE
        {0x25C0, 0x25C0},     // ; A  # So         BLACK LEFT-POINTING TRIANGLE
        {0x25C1, 0x25C1},     // ; A  # Sm         WHITE LEFT-POINTING TRIANGLE
        {0x25C6, 0x25C8},     // ; A  # So     [3] BLACK DIAMOND..WHITE DIAMOND CONTAINING BLACK SMALL DIAMOND
        {0x25CB, 0x25CB},     // ; A  # So         WHITE CIRCLE
        {0x25CE, 0x25D1},     // ; A  # So     [4] BULLSEYE..CIRCLE WITH RIGHT HALF BLACK
        {0x25E2, 0x25E5},     // ; A  # So     [4] BLACK LOWER RIGHT TRIANGLE..BLACK UPPER RIGHT TRIANGLE
        {0x25EF, 0x25EF},     // ; A  # So         LARGE CIRCLE
        {0x2605, 0x2606},     // ; A  # So     [2] BLACK STAR..WHITE STAR
        {0x2609, 0x2609},     // ; A  # So         SUN
        {0x260E, 0x260F},     // ; A  # So     [2] BLACK TELEPHONE..WHITE TELEPHONE
        {0x261C, 0x261C},     // ; A  # So         WHITE LEFT POINTING INDEX
        {0x261E, 0x261E},     // ; A  # So         WHITE RIGHT POINTING INDEX
        {0x2640, 0x2640},     // ; A  # So         FEMALE SIGN
        {0x2642, 0x2642},     // ; A  # So         MALE SIGN
        {0x2660, 0x2661},     // ; A  # So     [2] BLACK SPADE SUIT..WHITE HEART SUIT
        {0x2663, 0x2665},     // ; A  # So     [3] BLACK CLUB SUIT..BLACK HEART SUIT
        {0x2667, 0x266A},     // ; A  # So     [4] WHITE CLUB SUIT..EIGHTH NOTE
        {0x266C, 0x266D},     // ; A  # So     [2] BEAMED SIXTEENTH NOTES..MUSIC FLAT SIGN
        {0x266F, 0x266F},     // ; A  # Sm         MUSIC SHARP SIGN
        {0x269E, 0x269F},     // ; A  # So     [2] THREE LINES CONVERGING RIGHT..THREE LINES CONVERGING LEFT
        {0x26BF, 0x26BF},     // ; A  # So         SQUARED KEY
        {0x26C6, 0x26CD},     // ; A  # So     [8] RAIN..DISABLED CAR
        {0x26CF, 0x26D3},     // ; A  # So     [5] PICK..CHAINS
        {0x26D5, 0x26E1},     // ; A  # So    [13] ALTERNATE ONE-WAY LEFT WAY TRAFFIC..RESTRICTED LEFT ENTRY-2
        {0x26E3, 0x26E3},     // ; A  # So         HEAVY CIRCLE WITH STROKE AND TWO DOTS ABOVE
        {0x26E8, 0x26E9},     // ; A  # So     [2] BLACK CROSS ON SHIELD..SHINTO SHRINE
        {0x26EB, 0x26F1},     // ; A  # So     [7] CASTLE..UMBRELLA ON GROUND
        {0x26F4, 0x26F4},     // ; A  # So         FERRY
        {0x26F6, 0x26F9},     // ; A  # So     [4] SQUARE FOUR CORNERS..PERSON WITH BALL
        {0x26FB, 0x26FC},     // ; A  # So     [2] JAPANESE BANK SYMBOL..HEADSTONE GRAVEYARD SYMBOL
        {0x26FE, 0x26FF},     // ; A  # So     [2] CUP ON BLACK SQUARE..WHITE FLAG WITH HORIZONTAL MIDDLE BLACK STRIPE
        {0x273D, 0x273D},     // ; A  # So         HEAVY TEARDROP-SPOKED ASTERISK
        {0x2776, 0x277F},     // ; A  # No    [10] DINGBAT NEGATIVE CIRCLED DIGIT ONE..DINGBAT NEGATIVE CIRCLED NUMBER TEN
        {0x2B56, 0x2B59},     // ; A  # So     [4] HEAVY OVAL WITH OVAL INSIDE..HEAVY CIRCLED SALTIRE
        {0x3248, 0x324F},     // ; A  # No     [8] CIRCLED NUMBER TEN ON BLACK SQUARE..CIRCLED NUMBER EIGHTY ON BLACK SQUARE
        {0xE000, 0xF8FF},     // ; A  # Co  [6400] <private-use-E000>..<private-use-F8FF>
        {0xFE00, 0xFE0F},     // ; A  # Mn    [16] VARIATION SELECTOR-1..VARIATION SELECTOR-16
        {0xFFFD, 0xFFFD},     // ; A  # So         REPLACEMENT CHARACTER
        {0x1F100, 0x1F10A},   // ; A  # No    [11] DIGIT ZERO FULL STOP..DIGIT NINE COMMA
        {0x1F110, 0x1F12D},   // ; A  # So    [30] PARENTHESIZED LATIN CAPITAL LETTER A..CIRCLED CD
        {0x1F130, 0x1F169},   // ; A  # So    [58] SQUARED LATIN CAPITAL LETTER A..NEGATIVE CIRCLED LATIN CAPITAL LETTER Z
        {0x1F170, 0x1F18D},   // ; A  # So    [30] NEGATIVE SQUARED LATIN CAPITAL LETTER A..NEGATIVE SQUARED SA
        {0x1F18F, 0x1F190},   // ; A  # So     [2] NEGATIVE SQUARED WC..SQUARE DJ
        {0x1F19B, 0x1F1AC},   // ; A  # So    [18] SQUARED THREE D..SQUARED VOD
        {0xE0100, 0xE01EF},   // ; A  # Mn   [240] VARIATION SELECTOR-17..VARIATION SELECTOR-256
        {0xF0000, 0xFFFFD},   // ; A  # Co [65534] <private-use-F0000>..<private-use-FFFFD>
        {0x100000, 0x10FFFD}  // ; A  # Co [65534] <private-use-100000>..<private-use-10FFFD>
    };
    const size_t num_ranges = sizeof(ranges) / sizeof(ranges[0]);

    int left = 0, right = num_ranges - 1;

    while (left <= right) {
        int mid = left + (right - left) / 2;

        if (ucs >= ranges[mid].start && ucs <= ranges[mid].end) {
            return 1; // Character is in the range
        } else if (ucs < ranges[mid].start) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    return 0; // Character is not in any of the ranges
}

