uniform sampler2D currentTexture;

void main() {
    gl_FragColor = texture(currentTexture, gl_TexCoord[0].xy);
}
