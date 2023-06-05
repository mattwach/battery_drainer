use <mattwach/util.scad>
use <mattwach/honeycomb.scad>
include <heat_sink.scad>
include <main_pcb.scad>
include <NopSCADlib/core.scad>
include <NopSCADlib/vitamins/fans.scad>

cooling_fan_width = 120;
cooling_fan_height = 120;
cooling_fan_thickness = 25;
overlap = 0.01;

module cooling_fan() {
  fan(fan120x25);  
}

module placed_pcb() {
  pcb_xoffset = (heat_sink_xsize - main_pcb_length) / 2;
  pcb_yoffest = 20;
  translate([
      pcb_xoffset,
      -pcb_yoffest,
      0]) rx(90) main_pcb();
}

module placed_cooling_fan() {
  translate([
      heat_sink_xsize / 2,
      heat_sink_ysize + cooling_fan_thickness / 2,
      heat_sink_zsize / 2]) rx(90) cooling_fan();
}

module assembly() {
  heat_sink();
  placed_cooling_fan();
  placed_pcb();
}

module case() {
  case_xpad = 10;
  case_fan_pad = 3;
  case_bottom_pad = 10;
  case_xsize = heat_sink_xsize + case_xpad * 2;
  case_zsize = heat_sink_zsize + case_bottom_pad + case_fan_pad;

  module back_plate() {
    back_plate_thickness = 3;
    module plate() {
      cube([case_xsize, back_plate_thickness, case_zsize]);
    }

    module fan_hole() {
      fan_hole_diameter = cooling_fan_width - 2;
      honeycomb_size = 20;
      translate([
          case_xsize / 2,
          back_plate_thickness + overlap,
          case_bottom_pad + heat_sink_zsize / 2]) rx(90) intersection() {
        cylinder(d=fan_hole_diameter, h=back_plate_thickness + overlap * 2); 
        tz(-0.5) honeycomb([
            fan_hole_diameter * 1.3,
            fan_hole_diameter * 1.3,
            back_plate_thickness + 1], honeycomb_size, 2);
      }
    }

    difference() {
      plate();
      fan_hole();
    }
  }

  translate([
      -case_xpad,
      heat_sink_ysize + cooling_fan_thickness,
      -case_bottom_pad]) color("orange") back_plate();
}


$fa=2.0;
$fs=0.5;
assembly();
case();
