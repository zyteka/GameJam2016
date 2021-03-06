#include "Object.h"
#include "ObjLoader.h"

Object::Object()
{

	positionXYZ=glm::vec3(0);
	rotationXYZ = glm::vec3(0,1.0f,0);
	rotation=0.0f;
	sizeXYZ = glm::vec3(1);

	isStatic = false;
	isGhost = false;
	position = glm::mat4();
	fragmentName = "fragment-shader[basic].txt";
	displacementName = "gray.png";
	vertexName = "vertex-shader[basic].txt";
	textureName = "dirt.jpg";
	normalName = "black.png";
}


Object::~Object()
{
}

void Object::ExtractFromFile(const char* name){
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials;
	std::string err = tinyobj::LoadObj(shapes, materials, name, NULL);

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	uint overallSize = 0;
	uint faceOverallSize = 0;
	for (uint i = 0; i < shapes.size(); i++){
		overallSize += shapes[i].mesh.positions.size();
		faceOverallSize += shapes[i].mesh.indices.size();
	}



	uint overallOffset = 0;
	uint faceOffset = 0;

	for (uint i = 0; i < shapes.size(); i++){
		for (uint v = 0; v < overallSize / 3; v++){
			vertices.push_back({
				glm::vec3(shapes[i].mesh.positions[3 * v + 0], shapes[i].mesh.positions[3 * v + 1], shapes[i].mesh.positions[3 * v + 2]),
				glm::vec2(shapes[i].mesh.texcoords[2 * v + 0], shapes[i].mesh.texcoords[2 * v + 1]),
				glm::vec3(shapes[i].mesh.normals[3 * v + 0], shapes[i].mesh.normals[3 * v + 1], shapes[i].mesh.normals[3 * v + 2]) });
		}
		overallOffset += shapes[i].mesh.positions.size();

		for (uint f = 0; f < faceOverallSize / 3; f++){
			indices.push_back({
				glm::uvec3(shapes[i].mesh.indices[3 * f + 0], shapes[i].mesh.indices[3 * f + 1], shapes[i].mesh.indices[3 * f + 2]) });
				//shapes[i].mesh.material_ids[f] });
		}
		faceOffset += shapes[i].mesh.indices.size();
	}
}


void Object::Draw(Camera& camera)
{
	if (!isGhost){
		if (shader == NULL){
			std::cout << "Forgot to call Load()" << std::endl;
		}
		shader->use();
		glBindVertexArray(vao);
		shader->setUniform(cameraUniform, camera.matrix());
		shader->setUniform(texUniform, 0);
		shader->setUniform(disUniform, 1);
		shader->setUniform(normUniform, 2);
		shader->setUniform(posNormUniform, glm::inverse(glm::transpose(position)));
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture->object());

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, displacement->object());

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, normal->object());

		shader->setUniform(posUniform, position);
		glDrawElements(GL_TRIANGLES, (indices.size() * 3), GL_UNSIGNED_INT, (GLvoid*)0);
		glBindVertexArray(0);
		shader->stopUsing();
	}
}

void Object::NormalizeScale(glm::vec3 scale){
	for (int i = 0; i < GetVertices().size();i++){
		GetVertices()[i].position *=  scale;
	}
}
Bitmap Object::LoadBmp(const char* filename) {
	Bitmap bmp = Bitmap::bitmapFromFile(filename);
	bmp.flipVertically();
	return bmp;
}
Texture* Object::LoadTexture(Bitmap bmp){
	return new Texture
		(bmp);
}

void Object::Load(){

	shader = LoadShaders(vertexName, "geometry-shader[basic].txt", fragmentName);
	cameraUniform = shader->uniform("camera");
	posNormUniform = shader->uniform("normalPos");
	texUniform = shader->uniform("tex");
	disUniform = shader->uniform("dis");
	normUniform = shader->uniform("norm");
	posUniform = shader->uniform("position");
	normal = LoadTexture(LoadBmp(normalName));
	texture = LoadTexture(LoadBmp(textureName));
	displacement = LoadTexture(LoadBmp(displacementName));

	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &vertices.front(), GL_STATIC_DRAW);

	glEnableVertexAttribArray(shader->attrib("vert_VS_in"));
	glVertexAttribPointer(shader->attrib("vert_VS_in"), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), NULL);
	glEnableVertexAttribArray(shader->attrib("frag_VS_in"));
	glVertexAttribPointer(shader->attrib("frag_VS_in"), 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*)(3 * sizeof(GLfloat)));
	glEnableVertexAttribArray(shader->attrib("normal_VS_in"));
	glVertexAttribPointer(shader->attrib("normal_VS_in"), 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (const GLvoid*)(5 * sizeof(GLfloat)));

	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Face)*indices.size(), &indices.front(), GL_STATIC_DRAW);

	glBindVertexArray(0);
}
void Object::Flush(){
	glBindVertexArray(vao);

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex)*vertices.size(), &vertices.front(), GL_DYNAMIC_DRAW);

	glBindVertexArray(0);
}
void Object::Update(double dt){
	
}
void Object::UpdatePosition(){
	if (!isStatic){
		position = glm::translate(glm::mat4(), positionXYZ);
		position = glm::rotate(position, glm::radians(rotation), rotationXYZ);
		position = glm::scale(position, sizeXYZ);
	}
}
std::vector<Face>& Object::GetIndices(){
	return indices;
}
std::vector<Vertex>& Object::GetVertices(){
	return vertices;
}
glm::vec3 Object::GetPosition(){
	return positionXYZ;
}

glm::mat4 Object::GetMatrix(){
	return position;
}
