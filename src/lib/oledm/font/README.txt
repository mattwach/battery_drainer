I got this working in haste, then some years past and I wanted to add more fonts and
found the docs lacking.  Here are some after-the-fact tips.

There are a few file types here:
  .txt  A raw text representation of a font.  It looks like I used the psf2inc
  (in pstools package) to generate the terminius font.  Other fonts are hand-edited in
  a text editor.

To get from .txt to .c / .h, the make_fixed_ascii_font.py tool is used.  Below is a tested example.

  python3 ../tools/make_fixed_ascii_font.py terminus8x16.yaml

After that, there should be a .h and .c that you can use in your project.

There is also a scale_fixed_ascii_font.py which will output a scaled .txt from another one
I find it only does an "OK" job.  Anyway, it take additional parameters in the yaml to add
scaling parameters - see the source for details.

