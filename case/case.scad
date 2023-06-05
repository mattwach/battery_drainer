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
heat_sink_mount_xpad = 6;
case_xsize = heat_sink_xsize + heat_sink_mount_xpad * 2;
fan_shroud_gap = 1;

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

module backside() {
  case_fan_pad = 3;
  case_bottom_pad = case_fan_pad + fan_shroud_gap;
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

  module fan_shroud() {
    fan_shroud_inset = 20;
    fan_shroud_depth = 20;
    fan_shroud_width = cooling_fan_width + fan_shroud_gap + case_fan_pad * 2;
    fan_shroud_height = cooling_fan_height + fan_shroud_gap + case_fan_pad * 2;
    translate([
        (case_xsize - fan_shroud_width) / 2,
        -fan_shroud_depth,
        case_bottom_pad - case_fan_pad - fan_shroud_gap]) difference() {
      cube([
          fan_shroud_width,
          fan_shroud_depth + overlap,
          fan_shroud_height]);
      translate([
        case_fan_pad,
        -overlap,
        case_fan_pad + fan_shroud_gap
      ]) cube([
        fan_shroud_width - case_fan_pad * 2,
        fan_shroud_depth + overlap * 4,
        fan_shroud_height - case_fan_pad * 2]);
      translate([
        case_fan_pad + fan_shroud_inset,
        -overlap,
        -overlap
      ]) cube([
        fan_shroud_width - case_fan_pad * 2 - fan_shroud_inset * 2,
        fan_shroud_depth + overlap * 4,
        fan_shroud_height + overlap * 2]);
      translate([
        -overlap,
        -overlap,
        case_fan_pad + fan_shroud_gap + fan_shroud_inset
      ]) cube([
        fan_shroud_width + overlap * 2,
        fan_shroud_depth + overlap * 4,
        fan_shroud_height - case_fan_pad * 2 - fan_shroud_inset * 2]);
    }
  }

  module heat_sink_mount() {
    heat_sink_mount_inset = 5;
    heat_sink_mount_depth_inset = 0.3;
    heat_sink_mount_ipad = 0.5;
    heat_sink_mount_depth = cooling_fan_thickness + heat_sink_ysize - heat_sink_mount_depth_inset;
    translate([
        0,
        -heat_sink_mount_depth,
        case_bottom_pad - case_fan_pad]) difference() {
      cube([
          case_xsize,
          heat_sink_mount_depth + overlap,
          heat_sink_zsize + case_fan_pad * 2]);
      translate([
          heat_sink_mount_xpad - heat_sink_mount_ipad,
          -overlap,
          case_fan_pad - heat_sink_mount_ipad]) cube([
            heat_sink_xsize + heat_sink_mount_ipad * 2,
            heat_sink_mount_depth + overlap * 4,
            heat_sink_zsize + heat_sink_mount_ipad * 2]);
      translate([
          heat_sink_mount_xpad - heat_sink_mount_ipad + heat_sink_mount_inset,
          -overlap,
          -overlap]) cube([
            heat_sink_xsize + heat_sink_mount_ipad * 2 - heat_sink_mount_inset * 2,
            heat_sink_mount_depth + overlap * 4,
            heat_sink_zsize + heat_sink_mount_ipad * 2 + case_fan_pad * 2 + overlap * 2]);
      translate([
          -overlap,
          -overlap,
          case_fan_pad - heat_sink_mount_ipad + heat_sink_mount_inset]) cube([
            heat_sink_xsize + heat_sink_mount_ipad * 2 + heat_sink_mount_xpad * 2 + overlap * 2,
            heat_sink_mount_depth + overlap * 4,
            heat_sink_zsize + heat_sink_mount_ipad * 2 - heat_sink_mount_inset * 2]);
    }
  }

  translate([
      -heat_sink_mount_xpad,
      heat_sink_ysize + cooling_fan_thickness,
      -case_bottom_pad]) color("orange") union() {
    back_plate();
    fan_shroud();
    heat_sink_mount();
  }
}


$fa=2.0;
$fs=0.5;
assembly();
backside();
