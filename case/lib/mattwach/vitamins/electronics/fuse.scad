use <../../util.scad>

module glass_fuse(diameter, length, metal_length) {
  overlap = 0.01;
  shell = 0.5;
  module end() {
    color("silver") difference() {
      cylinder(d=diameter, h=metal_length);
      tz(shell) cylinder(d=diameter - shell * 2, h=metal_length);
    }
  }

  module glass() difference() {
    tz(metal_length) color("white", 0.2) difference() {
      cylinder(d=diameter, h=length - metal_length * 2);
      tz(-overlap) cylinder(
          d=diameter - shell * 2,
          h=length - metal_length * 2 + overlap * 2);
    }
  }

  module filament() {
    filament_width = 1;
    filament_thickness = 0.5;
    color("silver") translate([
        -filament_width / 2,
        -filament_thickness / 2,
        overlap]) cube([
          filament_width,
          filament_thickness,
          length - overlap * 2]);
  }

  filament();
  end();
  glass();
  tz(length) ry(180) end();
}
