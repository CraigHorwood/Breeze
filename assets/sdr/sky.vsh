precision highp float;
attribute vec2 a_pos;
uniform float u_scroll;
varying vec2 v_tex;
void main()
{
	vec2 uv = a_pos;
	uv.x *= 0.5625;
	uv.y = 1.0 - uv.y - u_scroll;
	v_tex = uv;
	gl_Position = vec4(a_pos * 2.0 - 1.0, 0.0, 1.0);
}