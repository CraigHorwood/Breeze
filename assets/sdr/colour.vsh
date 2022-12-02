precision highp float;
attribute vec2 a_pos;
uniform mat4 u_pr;
uniform mat4 u_mv;
void main()
{
	gl_Position = u_pr * u_mv * vec4(a_pos, 0.0, 1.0);
}