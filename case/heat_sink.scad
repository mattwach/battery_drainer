use <lib/mattwach/util.scad>

heat_sink_xsize = 138.9;
heat_sink_ysize = 36;
heat_sink_zsize = 120.3;

module heat_sink() {
  overlap = 0.01;
  module block() {
    cube([heat_sink_xsize, heat_sink_ysize, heat_sink_zsize]);
  }

  module fins() {
    fin_width = 1.5;
    fin_count = 52;
    fin_depth = heat_sink_ysize - 4.8;

    gap_space = fin_width * fin_count;
    fill_space = heat_sink_xsize - gap_space;
    fill_width = fill_space / (fin_count + 1);

    module fin() {
      translate([
          fill_width,
          heat_sink_ysize - fin_depth,
          -overlap]) cube([
            fin_width,
            fin_depth + overlap,
            heat_sink_zsize + overlap * 2]);
    }

    for (i = [0:fin_count-1]) {
      tx((fin_width + fill_width) * i) fin();
    }
  }

  difference() {
    block();
    color("silver") fins();
  }
}
