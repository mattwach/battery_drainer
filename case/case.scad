use <lib/mattwach/util.scad>
use <lib/mattwach/honeycomb.scad>
use <lib/mattwach/vitamins/bolts.scad>
include <heat_sink.scad>
include <main_pcb.scad>
include <lib/NopSCADlib/core.scad>
include <lib/NopSCADlib/vitamins/fans.scad>

back_plate_thickness = 3;
cooling_fan_width = 120;
cooling_fan_height = 120;
cooling_fan_thickness = 25;
overlap = 0.01;
heat_sink_mount_xpad = 8;
heat_sink_mount_inset = 5;
heat_sink_mount_depth_inset = 0.01;
heat_sink_mount_depth = cooling_fan_thickness + heat_sink_ysize - heat_sink_mount_depth_inset;
heat_sink_mount_ipad = 0.5;
fan_shroud_gap = 1;
glass_plate_thickness = 3.175;
case_fan_pad = 3;
case_bottom_pad = case_fan_pad + fan_shroud_gap;
case_xsize = heat_sink_xsize + heat_sink_mount_xpad * 2;
case_zsize = heat_sink_zsize + case_bottom_pad + case_fan_pad;
hole_inset_x = heat_sink_mount_xpad / 2;
hole_inset_z = (case_fan_pad + heat_sink_mount_inset + heat_sink_mount_ipad) / 2;
pcb_yoffset = 7;
pcd_plate_gap = 11;
main_plate_width = 9;
front_interface_thickness = 4;
front_foot_height = 10;
front_foot_width = heat_sink_mount_inset + heat_sink_mount_xpad - heat_sink_mount_ipad;
front_foot_depth = 2;
front_post_length = pcb_yoffset + pcd_plate_gap;


module cooling_fan() {
  fan(fan120x25);  
}

module placed_pcb() {
  pcb_xoffset = (heat_sink_xsize - main_pcb_length) / 2;
  translate([
      pcb_xoffset,
      -pcb_yoffset - 6,
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

module frontside_mounting_holes(hole_diameter=2.85, hole_depth=15) {
  module hole() {
    ty(-overlap) rx(-90) cylinder(d=hole_diameter, h=hole_depth + overlap);
  }

  module row() {
    tx(hole_inset_x) hole();
    tx(case_xsize - hole_inset_x) hole();
  }

  tz(hole_inset_z) row();
  tz(case_zsize - hole_inset_z) row();
}

module backside() {
  module back_plate() {
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
    fan_shroud_xextend = 24;
    translate([
        (case_xsize - fan_shroud_width) / 2,
        -fan_shroud_depth,
        case_bottom_pad - case_fan_pad - fan_shroud_gap]) difference() {
      tx(-fan_shroud_xextend / 2) cube([
          fan_shroud_width + fan_shroud_xextend,
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
        -overlap - fan_shroud_xextend / 2,
        -overlap,
        case_fan_pad + fan_shroud_gap + fan_shroud_inset
      ]) cube([
        fan_shroud_width + fan_shroud_xextend + overlap * 2,
        fan_shroud_depth + overlap * 4,
        fan_shroud_height - case_fan_pad * 2 - fan_shroud_inset * 2]);
    }
  }

  module heat_sink_mount() {
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
      frontside_mounting_holes();
    }
  }

  module back_feet() {
    module foot() {
      translate([
          0,
          -heat_sink_mount_depth,
          case_bottom_pad - case_fan_pad - front_foot_height]) cube([
            front_foot_width,
            heat_sink_mount_depth + back_plate_thickness,
            front_foot_height + overlap]);
    }

    foot();
    tx(case_xsize - front_foot_width) foot();
  }

  translate([
      -heat_sink_mount_xpad,
      heat_sink_ysize + cooling_fan_thickness,
      -case_bottom_pad]) color("orange") union() {
    back_plate();
    fan_shroud();
    heat_sink_mount();
    back_feet();
  }
}

module frontside() {
  module interface_feet() {
    foot_xsize = heat_sink_mount_xpad + heat_sink_mount_inset;
    foot_zsize = case_fan_pad + heat_sink_mount_inset;
    module foot() {
      translate([
          -heat_sink_mount_xpad,
          -front_foot_depth,
          -case_fan_pad]) cube([
            foot_xsize,
            front_foot_depth,
            foot_zsize]);
    }

    foot();
    tx(case_xsize - foot_xsize) foot();
    tz(case_zsize - foot_zsize) foot();
    tx(case_xsize - foot_xsize) tz(case_zsize - foot_zsize) foot();
  }

  module main_plate() {
    translate([
        -heat_sink_mount_xpad,
        -front_foot_depth - front_interface_thickness + overlap,
        -case_fan_pad]) difference() {
      cube([
          case_xsize,
          front_interface_thickness + overlap,
          case_zsize]);
      translate([
          main_plate_width,
          -overlap * 6,
          -overlap]) cube([
            case_xsize - main_plate_width * 2,
            front_interface_thickness + overlap * 12,
            case_zsize - main_plate_width + overlap]);
    }
  }

  module front_plate_mounting_posts() {
    module post() {
      translate([
          -heat_sink_mount_xpad,
          -front_foot_depth - front_interface_thickness - front_post_length + overlap,
          -case_fan_pad]) cube([
            front_foot_width,
            front_post_length + front_interface_thickness + overlap,
            main_plate_width]);
    }

    post();
    tx(case_xsize - front_foot_width) post();
    tz(case_zsize - main_plate_width) post();
    tx(case_xsize - front_foot_width) tz(case_zsize - main_plate_width) post();
  }

  module front_feet() {
    module foot() {
      translate([
          -heat_sink_mount_xpad,
          -front_foot_depth - front_interface_thickness - front_post_length,
          -case_fan_pad - front_foot_height]) cube([
            front_foot_width,
            front_post_length + front_interface_thickness + front_foot_depth,
            front_foot_height + overlap]);
    }

    foot();
    tx(case_xsize - front_foot_width) foot();
  }

  module fan_cable_tiedown() {
    hole_span = 12;
    module hole() {
      hole_width = 3.5;
      hole_height = 2.25;
      hole_z_offset = 75;
      translate([
          -heat_sink_mount_xpad + (main_plate_width - hole_width) / 2,
          -front_foot_depth - front_interface_thickness,
          hole_z_offset]) cube([hole_width, front_interface_thickness + overlap * 3, hole_height]);
    }

    tz(-hole_span / 2) hole();
    tz(hole_span / 2) hole();
  }

  color("#f60") difference() {
    union() {
      interface_feet();
      main_plate();
      front_plate_mounting_posts();
      front_feet();
    }
    translate([
        -heat_sink_mount_xpad,
        -front_interface_thickness - front_foot_depth - front_post_length - overlap,
        -case_bottom_pad]) frontside_mounting_holes(
          hole_diameter=3.2,
          hole_depth=front_interface_thickness + front_foot_depth + front_post_length + overlap * 4);
    fan_cable_tiedown();
  }
}

module glass_plate() {
  module plate() {
    cube([case_xsize, glass_plate_thickness, case_zsize]);
  }

  module bolt_holes() {
    translate([
        0,
        -overlap,
        -fan_shroud_gap]) frontside_mounting_holes(
          hole_diameter=3.2,
          hole_depth=glass_plate_thickness + overlap * 2);
  }

  module finger_button() {
    diameter = 15;
    ty(-overlap) rx(-90) cylinder(h=glass_plate_thickness + overlap * 2, d=diameter);
  }

  module pen_button() {
    diameter = 8;
    ty(-overlap) rx(-90) cylinder(h=glass_plate_thickness + overlap * 2, d=diameter);
  }

  module interface_buttons() {
    iface_length = 29.5;
    iface_xoffset = 84.5;
    iface_zoffset = 17.5;
    translate([
        iface_xoffset,
        0,
        iface_zoffset]) {
      hull() {
        finger_button();
        tx(iface_length) finger_button();
      }
    }
  }

  module reset_button() {
    reset_xoffset = 44.5;
    reset_zoffset = 50.5;
    translate([
        reset_xoffset,
        0,
        reset_zoffset]) pen_button();
  }

  module boot_button() {
    boot_xoffset = 29.7;
    boot_zoffset = 21;
    translate([
        boot_xoffset,
        0,
        boot_zoffset]) pen_button();
  }

  module fuse_slot() {
    slot_xoffset = 131.2;
    slot_zoffset = 35;
    slot_width = 10;
    slot_length = 31.7;
    module end() {
      rx(-90) cylinder(d=slot_width, h=glass_plate_thickness + overlap * 2);
    }
    translate([
        slot_xoffset,
        -overlap,
        slot_zoffset]) hull() {
      end();
      tz(slot_length) end();
    }
  }

  module xt60_slot() {
    fillet = 5;
    slot_zsize = 18;
    slot_xsize = 20;
    slot_zoffset = 12.5;
    module end() {
      rx(-90) cylinder(r=fillet, h=glass_plate_thickness + overlap * 2);
    }
    translate([
        case_xsize - slot_xsize,
        -overlap,
        slot_zoffset]) hull() {
      translate([fillet, 0, fillet]) end();
      translate([fillet, 0, slot_zsize - fillet]) end();
      tx(slot_xsize - 1) cube([1 + overlap, glass_plate_thickness + overlap * 2, slot_zsize]);
    }
  }

  color("#f08", 0.25) translate([
      -heat_sink_mount_xpad,
      -front_foot_depth - front_interface_thickness - front_post_length - glass_plate_thickness,
      -case_fan_pad]) difference() {
    plate();
    bolt_holes();
    interface_buttons();
    reset_button();
    boot_button();
    fuse_slot();
    xt60_slot();
  }
}

module bolts() {
  module hole() {
    ty(-overlap) rx(90) bolt_M3(30);
  }

  module row() {
    tx(hole_inset_x) hole();
    tx(case_xsize - hole_inset_x) hole();
  }

  translate([
      -heat_sink_mount_xpad,
      -front_foot_depth - front_interface_thickness - front_post_length - glass_plate_thickness,
      -case_fan_pad - 1]) {
    tz(hole_inset_z) row();
    tz(case_zsize - hole_inset_z) row();
  }
}

module glass_plate_projection() {
  projection() rx(-90) glass_plate();
}


$fa=2.0;
$fs=0.5;
assembly();
backside();
frontside();
bolts();
glass_plate();
//glass_plate_projection();

