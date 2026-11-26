"""Broken CR2 files, for the darkroom to turn down: each is a good one
(mkcr2's) broken as a damaged card or file can be: in its RAW's lossless
JPEG header, in the TIFF around the RAW, or in the RAW's data. The darkroom
must say so, and not read or write outside what it has, nor go round
forever (cardtest.sh).

    python3 badcr2.py good.CR2 outdir

Writes outdir/<name>.CR2 for each below and outdir/LIST.txt: a line for
each, its name and what the darkroom must do: give that message; or develop,
every sample right ("ok": the header taken apart and put back as it was, or
changed in a way it must take); or develop as far as the file goes, its
samples not checked ("develops": the RAW cut short, decoded to nonsense, or
read past its end). The good frame must be as mkcr2 writes it, with --wb:
its RAW last, and the camera's white balance.
"""
import struct
import sys
from pathlib import Path

WINDOW = 2048   # the bytes of the header the darkroom reads at once (fos_dev.c, parse_ljpeg)
RAW_MAX = 16384  # the widest and tallest RAW frame it takes (fos_dev.c, setup_maps)
DHT, SOF3, DRI, SOS, COM = 0xC4, 0xC3, 0xDD, 0xDA, 0xFE


def raw_strip(b):
    """the RAW's offset and length, and where its length is written (the RAW IFD's 0x0111, 0x0117)"""
    ifd = struct.unpack_from("<I", b, 12)[0]
    off = ln = at = None
    for i in range(struct.unpack_from("<H", b, ifd)[0]):
        e = ifd + 2 + 12 * i
        tag = struct.unpack_from("<H", b, e)[0]
        if tag == 0x0111:
            off = struct.unpack_from("<I", b, e + 8)[0]
        elif tag == 0x0117:
            ln, at = struct.unpack_from("<I", b, e + 8)[0], e + 8
    return off, ln, at


def split(raw):
    """the segments, the scan's header the last, as [tag, body]; and the entropy data after them"""
    if raw[:2] != b"\xff\xd8":
        sys.exit("the RAW is not a lossless JPEG")
    segs, p = [], 2
    while True:
        tag, n = raw[p + 1], raw[p + 2] << 8 | raw[p + 3]
        segs.append([tag, bytearray(raw[p + 4:p + 2 + n])])
        p += 2 + n
        if tag == SOS:
            return segs, raw[p:]


def join(segs):
    out = bytearray(b"\xff\xd8")
    for tag, body in segs:
        out += bytes([0xFF, tag]) + struct.pack(">H", len(body) + 2) + body
    return out


def with_raw(b, off, at, raw):
    """the file with its RAW (the last thing in it) changed to raw, and the RAW's length with it"""
    f = bytearray(b[:off]) + raw
    struct.pack_into("<I", f, at, len(raw))
    return f


def dhts(segs):
    return [body for tag, body in segs if tag == DHT]


def body_of(segs, tag):
    return next(body for t, body in segs if t == tag)


def free_codes(counts):
    """a table's longest code length, and how many codes of that length its codes leave free"""
    longest = max(n for n in range(1, 17) if counts[n - 1])
    return longest, (1 << longest) - sum(c << (longest - n) for n, c in enumerate(counts[:longest], 1))


def add_table(segs, counts, values):
    """a segment of one more table, number 2, after the others: in a frame
    of two components (mkcr2's), one no component takes"""
    last = max(i for i, (tag, _) in enumerate(segs) if tag == DHT)
    segs.insert(last + 1, [DHT, bytearray([2] + counts + values)])
    return segs


# mkcr2 writes a table to a segment: its number, its 16 counts (codes of
# each length), its values (the difference sizes)

def codes_overflow(segs):
    """more codes of a length than it has room for (fifteen 1-bit codes): the
    lookup the darkroom fills for them would run past its end"""
    t = dhts(segs)[0]
    total = sum(t[1:17])
    t[1:17] = bytes([total] + [0] * 15)
    return segs


def codes_one_over(segs):
    """one code more than there is room for, at 9 bits, the lookup's own
    length: a code of each length from 1 to 8 leaves room for two of 9 bits,
    and it has three (the third's would be written one past the lookup's
    end). With two, its codes would fill their lengths, as complete's do"""
    t = dhts(segs)[0]
    t[1:] = bytes([1] * 8 + [3] + [0] * 7 + list(range(11)))
    return segs


def codes_over_at_16(segs):
    """the same at 16 bits, the longest a code is, past the lookup: a code
    of each length from 1 to 15 leaves room for two of 16 bits, and it has
    three"""
    t = dhts(segs)[0]
    t[1:] = bytes([1] * 15 + [3] + [i % 17 for i in range(18)])
    return segs


def values_cut(segs):
    """the segment ends before the table's last 5 values"""
    del dhts(segs)[0][-5:]
    return segs


def counts_cut(segs):
    """the segment ends in the middle of the 16 counts"""
    del dhts(segs)[0][9:]
    return segs


def values_over_256(segs):
    """300 values (codes 15 and 16 bits long, room for all): no table holds more than 256"""
    t = dhts(segs)[0]
    t[1:] = bytes([0] * 14 + [45, 255] + [i % 15 for i in range(300)])
    return segs


def values_257(segs):
    """a third table of 257 values, one more than a table holds: 255 codes
    of 8 bits and two of 9, which fill their lengths"""
    return add_table(segs, [0] * 7 + [255, 2] + [0] * 7, [i % 17 for i in range(257)])


def values_256(segs):
    """a third table of 256 values, as many as a table holds: 255 codes of
    8 bits and one of 9"""
    return add_table(segs, [0] * 7 + [255, 1] + [0] * 7, [i % 17 for i in range(256)])


def codes_at_16(segs):
    """a third table with a code of each length from 1 to 15 and two of 16
    bits, the longest a code is: they fill their lengths, as the three of
    codes-over-at-16 can't"""
    return add_table(segs, [1] * 15 + [2], [i % 17 for i in range(17)])


def value_17(segs):
    """a value no difference has: they go up to 16"""
    dhts(segs)[0][-1] = 17
    return segs


def value_16(segs):
    """the first table given one of the codes it leaves free, of value 16,
    the largest a difference has (-32768, with no bits after its code): the
    data never uses it"""
    t = dhts(segs)[0]
    counts = list(t[1:17])
    longest, room = free_codes(counts)
    if not room or counts[longest - 1] == 255 or sum(counts) == 256:
        sys.exit("the first table leaves no code free: not one to add to")
    counts[longest - 1] += 1
    t[1:17] = bytes(counts)
    t.append(16)
    return segs


def dht_extra_byte(segs):
    """the first table's segment a byte longer than its table: the start of
    another, cut short"""
    dhts(segs)[0].append(0)
    return segs


def table_empty(segs):
    """the second table with no codes at all, nothing to decode with"""
    dhts(segs)[1][1:] = bytes(16)
    return segs


def table_gap(segs):
    """tables 0 and 2 and no 1, which the second component takes: it was
    decoded with a table of no codes"""
    dhts(segs)[1][0] = 2
    return segs


def only_table_1(segs):
    """table 1 alone: the first component takes table 0, which isn't there"""
    return [s for s in segs if s[0] != DHT or s[1][0] != 0]


def window_end(segs):
    """the second table's segment ends where the bytes read at once do, 5 of
    its values short: they would be read past those bytes"""
    t0, t1 = dhts(segs)
    del t1[-5:]
    fill = WINDOW - 2 - (4 + len(t0)) - (4 + len(t1)) - 4
    rest = [s for s in segs if s[0] != DHT]
    return [[DHT, t0], [COM, bytearray(fill)], [DHT, t1]] + rest


def sos_components(segs):
    """the scan's header names 255 components, and holds 2"""
    body_of(segs, SOS)[0] = 255
    return segs


def sof_short(segs):
    """the frame's header ends after the number of components"""
    del body_of(segs, SOF3)[6:]
    return segs


def dri_short(segs):
    """a restart interval with no interval in it"""
    return [[DRI, bytearray()]] + segs


def sof_wide_0(segs):
    """a frame of no width"""
    body_of(segs, SOF3)[3:5] = bytes(2)
    return segs


def sof_high_0(segs):
    """a frame of no height: its slices still add up"""
    body_of(segs, SOF3)[1:3] = bytes(2)
    return segs


def sof_too_wide(segs):
    """a frame 65535 samples wide in each component, wider than any sensor"""
    body_of(segs, SOF3)[3:5] = b"\xff\xff"
    return segs


def sof_too_high(segs):
    """a frame 65535 rows high, higher than any sensor: its slices still add
    up, and the darkroom went through 86 million samples, nearly all of them
    past the file's end"""
    body_of(segs, SOF3)[1:3] = b"\xff\xff"
    return segs


def same(segs):
    return segs


def complete(segs):
    """each table given the codes it leaves free at its longest length (the
    all-ones code, or more), with value 0: the data never uses them"""
    for t in dhts(segs):
        counts = list(t[1:17])
        longest, room = free_codes(counts)
        # no table holds more than 256 values, nor a count more than 255 codes
        if not 0 < room <= 256 - sum(counts) or counts[longest - 1] + room > 255:
            sys.exit(f"a table with {room} codes free: not one to complete")
        counts[longest - 1] += room
        t[1:17] = bytes(counts)
        t += bytes(room)
    return segs


# The whole file: these take it and change it, most of them in the TIFF
# around the RAW. Its offsets are 32 bits; the darkroom read them as longs,
# which are 64 bits on a PC and 32 on the camera, so a damaged one did one
# thing on each.

def u32(b, at):
    return struct.unpack_from("<I", b, at)[0]


def tag_at(b, ifd, tag):
    """where the value of tag is in the IFD at ifd (the entry's last 4 bytes)"""
    for i in range(struct.unpack_from("<H", b, ifd)[0]):
        e = ifd + 2 + 12 * i
        if struct.unpack_from("<H", b, e)[0] == tag:
            return e + 8
    sys.exit(f"no tag {tag:#06x} in the IFD at {ifd}: wants a frame as mkcr2 --wb writes it")


def raw_ifd(b):
    return u32(b, 12)


def maker_note(b):
    exif = u32(b, tag_at(b, u32(b, 4), 0x8769))
    return u32(b, tag_at(b, exif, 0x927C))


def slices_at(f):
    """where the RAW's slices are (0xC640): how many, their width, the last's width"""
    return u32(f, tag_at(f, raw_ifd(f), 0xC640))


def slices_width_0(f):
    """one slice of no width and a last as wide as the frame: they add up,
    and the darkroom went round forever on the first"""
    s = slices_at(f)
    n, w, last = struct.unpack_from("<3H", f, s)
    struct.pack_into("<3H", f, s, 1, 0, n * w + last)
    return f


def slices_overflow(f):
    """65535 slices of 65535 columns: their sum was past what an int holds"""
    struct.pack_into("<3H", f, slices_at(f), 65535, 65535, 0)
    return f


def slices_last_0(f):
    """the slices as they are, but the last as wide as the others written as
    one of them, and a last of no width after it: nothing left to decode"""
    s = slices_at(f)
    n, w, last = struct.unpack_from("<3H", f, s)
    if last != w:
        sys.exit("the last slice isn't as wide as the others")
    struct.pack_into("<3H", f, s, n + 1, w, 0)
    return f


def most_samples(f, more=0):
    """the frame 16384 rows high and 4096 samples wide, with a column of
    each component more for each of more, in no slices but the last
    (0, 0, its width), as some cameras write it; the RAW's data as it was"""
    off, _, at = raw_strip(f)
    segs, data = split(f[off:])
    sof = body_of(segs, SOF3)
    clrs = sof[5]
    if 4096 % clrs:
        sys.exit(f"{clrs} components: 4096 samples aren't a whole number of columns of each")
    wide = 4096 // clrs + more
    struct.pack_into(">HH", sof, 1, RAW_MAX, wide)
    f = with_raw(f, off, at, join(segs) + data)
    struct.pack_into("<3H", f, slices_at(f), 0, 0, wide * clrs)
    return f


def samples_most(f):
    """the tallest frame taken, of the most samples taken: 67 million (the
    5DS's are 53). It develops, nearly all of it past the file's end"""
    return most_samples(f)


def samples_too_many(f):
    """the same and a column of each component more: more samples than the
    darkroom takes, which would keep the camera decoding for minutes"""
    return most_samples(f, 1)


def widest(f, more=0):
    """the frame 16384 samples wide, with a sample more for each of more:
    in two components, a frame twice that wide and half as high, which the
    darkroom folds back (as some cameras write it), so a side can be an odd
    number of samples; in no slices but the last (0, 0, its width); the
    RAW's data as it was"""
    off, _, at = raw_strip(f)
    segs, data = split(f[off:])
    sof = body_of(segs, SOF3)
    if sof[5] != 2:
        sys.exit(f"{sof[5]} components: wants mkcr2's 2, to fold the frame")
    high = struct.unpack_from(">H", sof, 1)[0]
    wide = RAW_MAX + more
    struct.pack_into(">HH", sof, 1, high // 2, wide)
    f = with_raw(f, off, at, join(segs) + data)
    struct.pack_into("<3H", f, slices_at(f), 0, 0, wide)
    return f


def width_16384(f):
    """the widest frame taken. It develops, most of it past the file's end"""
    return widest(f)


def width_16384_no_crop(f):
    """the same, its sensor's picture area 4 GB into the file: the darkroom
    works one out from the frame's size, 16087 columns wide (mkcr2's has
    1296, at the left)"""
    return sensor_info_huge(widest(f))


def width_16384_edges(f):
    """the same, its sensor's picture area as wide as the frame: its first
    and last columns print as well (the one worked out leaves them out)"""
    f = widest(f)
    si = u32(f, tag_at(f, maker_note(f), 0x00E0))
    struct.pack_into("<H", f, si + 10, 0)
    struct.pack_into("<H", f, si + 14, RAW_MAX - 1)
    return f


def width_16385(f):
    """a sample wider than the darkroom takes"""
    return widest(f, 1)


def raw_ifd_huge(f):
    """the RAW's IFD 4 GB into the file"""
    struct.pack_into("<I", f, 12, 0xFFFFFFF0)
    return f


def raw_offset_huge(f):
    """the RAW 4 GB into the file: on the camera, no RAW at all"""
    struct.pack_into("<I", f, tag_at(f, raw_ifd(f), 0x0111), 0xFFFFFF00)
    return f


def raw_length_huge(f):
    """the RAW 4 GB long, so up to the file's end: on a PC its end wrapped
    round to before its start, and the frame failed"""
    struct.pack_into("<I", f, tag_at(f, raw_ifd(f), 0x0117), 0xFFFFFFFF)
    return f


def raw_length_short(f):
    """the RAW half as long as it is: nothing read after that"""
    at = tag_at(f, raw_ifd(f), 0x0117)
    struct.pack_into("<I", f, at, u32(f, at) // 2)
    return f


def file_cut(f):
    """the file cut off halfway through the RAW, its length as it was (a
    copy that stopped)"""
    off, ln, _ = raw_strip(f)
    return f[:off + ln // 2]


def lone_ff(f):
    """the scan's data a single byte, 0xFF, the file's last: the darkroom
    looked for the byte after it past what it had read (on a PC it crashed)"""
    off, _, at = raw_strip(f)
    segs, _ = split(f[off:])
    return with_raw(f, off, at, join(segs) + b"\xff")


def ifd0_huge(f):
    """the first IFD 4 GB into the file: no orientation, no maker note"""
    struct.pack_into("<I", f, 4, 0xFFFFFFF0)
    return f


def sensor_info_huge(f):
    """the sensor's picture area 4 GB into the file"""
    struct.pack_into("<I", f, tag_at(f, maker_note(f), 0x00E0), 0xFFFFFF00)
    return f


def colour_data_huge(f):
    """the camera's colour data just short of 2 GB into the file: its white
    balance, 126 bytes into it, past 2 GB, where an int ran over"""
    struct.pack_into("<I", f, tag_at(f, maker_note(f), 0x4001), 0x7FFFFFF0)
    return f


def crop_upside_down(f):
    """the picture's bottom row above its top: a picture of less than no
    height (and a bigger print's memory of less than none)"""
    si = u32(f, tag_at(f, maker_note(f), 0x00E0))
    top = struct.unpack_from("<H", f, si + 12)[0]
    struct.pack_into("<H", f, si + 16, top - 2)
    return f


def orientation_bad(f):
    """an orientation no camera writes"""
    struct.pack_into("<H", f, tag_at(f, u32(f, 4), 0x0112), 9)
    return f


def samples_wrap_wb_8x(f):
    """every difference 15 bits long, so the samples come out anywhere in
    their 16 bits, and red and blue balanced 8 times green, the most taken:
    a print pixel's mean times its balance ran past what an int holds"""
    off, _, at = raw_strip(f)
    segs, data = split(f[off:])
    for t in dhts(segs):
        t[17:] = bytes([15] * (len(t) - 17))
    f = with_raw(f, off, at, join(segs) + data)
    cd = u32(f, tag_at(f, maker_note(f), 0x4001))
    g = struct.unpack_from("<H", f, cd + 2 * 64)[0]
    struct.pack_into("<H", f, cd + 2 * 63, 8 * g)
    struct.pack_into("<H", f, cd + 2 * 66, 8 * g)
    return f


BAD = "bad Huffman table"
BROKEN = "broken RAW header"
SIZE = "unsupported RAW size"
JPEG_WAYS = [
    ("codes-overflow", codes_overflow, BAD),
    ("codes-one-over", codes_one_over, BAD),
    ("codes-over-at-16", codes_over_at_16, BAD),
    ("values-cut", values_cut, BAD),
    ("counts-cut", counts_cut, BAD),
    ("values-over-256", values_over_256, BAD),
    ("values-257", values_257, BAD),
    ("value-17", value_17, BAD),
    ("dht-extra-byte", dht_extra_byte, BAD),
    ("table-empty", table_empty, BAD),
    ("table-gap", table_gap, BAD),
    ("only-table-1", only_table_1, BAD),
    ("window-end", window_end, BAD),
    ("sos-components", sos_components, BROKEN),
    ("sof-short", sof_short, BROKEN),
    ("dri-short", dri_short, BROKEN),
    ("sof-wide-0", sof_wide_0, SIZE),
    ("sof-high-0", sof_high_0, SIZE),
    ("sof-too-wide", sof_too_wide, SIZE),
    ("sof-too-high", sof_too_high, SIZE),
    ("same", same, "ok"),
    ("complete", complete, "ok"),
    ("values-256", values_256, "ok"),
    ("value-16", value_16, "ok"),
    ("codes-at-16", codes_at_16, "ok"),
]
FILE_WAYS = [
    ("slices-width-0", slices_width_0, "RAW slices of width 0"),
    ("slices-overflow", slices_overflow, "RAW slices don't add up"),
    ("slices-last-0", slices_last_0, "ok"),
    ("samples-most", samples_most, "develops"),
    ("samples-too-many", samples_too_many, SIZE),
    ("width-16384", width_16384, "develops"),
    ("width-16384-no-crop", width_16384_no_crop, "develops"),
    ("width-16384-edges", width_16384_edges, "develops"),
    ("width-16385", width_16385, SIZE),
    ("raw-ifd-huge", raw_ifd_huge, "no RAW in this file"),
    ("raw-offset-huge", raw_offset_huge, "can't read the RAW"),
    ("raw-length-huge", raw_length_huge, "ok"),
    ("raw-length-short", raw_length_short, "develops"),
    ("file-cut", file_cut, "develops"),
    ("lone-ff", lone_ff, "develops"),
    ("ifd0-huge", ifd0_huge, "ok"),
    ("sensor-info-huge", sensor_info_huge, "ok"),
    ("colour-data-huge", colour_data_huge, "ok"),
    ("crop-upside-down", crop_upside_down, "ok"),
    ("orientation-bad", orientation_bad, "ok"),
    ("samples-wrap-wb-8x", samples_wrap_wb_8x, "develops"),
]


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    b = Path(sys.argv[1]).read_bytes()
    off, ln, at = raw_strip(b)
    if off is None or ln is None or off + ln != len(b):
        sys.exit(f"{sys.argv[1]}: wants the RAW last in the file, as mkcr2 writes it")
    out = Path(sys.argv[2])
    out.mkdir(parents=True, exist_ok=True)
    files = []
    for name, way, want in JPEG_WAYS:
        segs, data = split(b[off:])
        files.append((name, with_raw(b, off, at, join(way(segs)) + data), want))
    for name, way, want in FILE_WAYS:
        files.append((name, way(bytearray(b)), want))
    for name, f, _ in files:
        (out / f"{name}.CR2").write_bytes(f)
    (out / "LIST.txt").write_text("".join(f"{name} {want}\n" for name, _, want in files))


if __name__ == "__main__":
    main()
