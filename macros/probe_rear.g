#<fast_rate>=140
#<slow_rate>=20
#<probe_dist>=60
#<probe_offset>=10.39 ; probe offset
#<retract_dist>=5

G91
G38.2 Y[-#<probe_dist>] F#<fast_rate> ; probe fast
G1 Y1 F1000  ; retract a little
G38.2 Y[-#<probe_dist>] F#<slow_rate> ; probe slowly
G90
G10 L20 P0 Y[-#<probe_offset>]
$J=G91 G21 F1000 Y[#<retract_dist>]
