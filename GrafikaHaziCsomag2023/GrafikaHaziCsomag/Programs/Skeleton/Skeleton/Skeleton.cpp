//=============================================================================================
// Mintaprogram: Zöld háromszög. Ervenyes 2019. osztol.
//
// A beadott program csak ebben a fajlban lehet, a fajl 1 byte-os ASCII karaktereket tartalmazhat, BOM kihuzando.
// Tilos:
// - mast "beincludolni", illetve mas konyvtarat hasznalni
// - faljmuveleteket vegezni a printf-et kiveve
// - Mashonnan atvett programresszleteket forrasmegjeloles nelkul felhasznalni es
// - felesleges programsorokat a beadott programban hagyni!!!!!!! 
// - felesleges kommenteket a beadott programba irni a forrasmegjelolest kommentjeit kiveve
// ---------------------------------------------------------------------------------------------
// A feladatot ANSI C++ nyelvu forditoprogrammal ellenorizzuk, a Visual Studio-hoz kepesti elteresekrol
// es a leggyakoribb hibakrol (pl. ideiglenes objektumot nem lehet referencia tipusnak ertekul adni)
// a hazibeado portal ad egy osszefoglalot.
// ---------------------------------------------------------------------------------------------
// A feladatmegoldasokban csak olyan OpenGL fuggvenyek hasznalhatok, amelyek az oran a feladatkiadasig elhangzottak 
// A keretben nem szereplo GLUT fuggvenyek tiltottak.
//
// NYILATKOZAT
// ---------------------------------------------------------------------------------------------
// Nev    : Erdei Dániel Patrik
// Neptun : BAVX18
// ---------------------------------------------------------------------------------------------
// ezennel kijelentem, hogy a feladatot magam keszitettem, es ha barmilyen segitseget igenybe vettem vagy
// mas szellemi termeket felhasznaltam, akkor a forrast es az atvett reszt kommentekben egyertelmuen jeloltem.
// A forrasmegjeloles kotelme vonatkozik az eloadas foliakat es a targy oktatoi, illetve a
// grafhazi doktor tanacsait kiveve barmilyen csatornan (szoban, irasban, Interneten, stb.) erkezo minden egyeb
// informaciora (keplet, program, algoritmus, stb.). Kijelentem, hogy a forrasmegjelolessel atvett reszeket is ertem,
// azok helyessegere matematikai bizonyitast tudok adni. Tisztaban vagyok azzal, hogy az atvett reszek nem szamitanak
// a sajat kontribucioba, igy a feladat elfogadasarol a tobbi resz mennyisege es minosege alapjan szuletik dontes.
// Tudomasul veszem, hogy a forrasmegjeloles kotelmenek megsertese eseten a hazifeladatra adhato pontokat
// negativ elojellel szamoljak el es ezzel parhuzamosan eljaras is indul velem szemben.
//=============================================================================================
#include "framework.h"


const char* const vertexSource = R"(
	#version 330				// Shader 3.3
	precision highp float;		// normal floats, makes no difference on desktop computers

	uniform mat4 MVP;			// uniform variable, the Model-View-Projection transformation matrix
	layout(location = 0) in vec2 vp;	// Varying input: vp = vertex position is expected in attrib array 0

	void main() {
		//gl_Position = vec4(vp.x, vp.y, 0, 1) * MVP;		// transform vp from modeling space to normalized device space
		float w = sqrt(dot(vp, vp)+1.0f);
		vec2 hyperbolic = (2.0 * vp) / (1.0 + dot(vp, vp));
        gl_Position = vec4(hyperbolic.x/(w*0.01+1), hyperbolic.y/(w*0.01+1), 0, 1) *MVP;
	}
)";

const char* const fragmentSource = R"(
	#version 330			// Shader 3.3
	precision highp float;	// normal floats, makes no difference on desktop computers
	
	uniform vec3 color;		// uniform variable, the color of the primitive
	out vec4 outColor;		// computed color of the current pixel

	void main() {
		outColor = vec4(color, 1);	// computed color is the color of the primitive
	}
)";

GPUProgram gpuProgram;
const int nv = 100;

class Line {
	unsigned int  vao, vbo;
public:
	std::vector<vec2> points;
	Line(vec2 p) {
		points.push_back(p);
	}

	void Create() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		glEnableVertexAttribArray(0);

		glBufferData(GL_ARRAY_BUFFER,
			sizeof(vec2) * points.size(),
			points.data(),
			GL_STATIC_DRAW);

		glVertexAttribPointer(0,
			2, GL_FLOAT, GL_FALSE,
			0, NULL);
	}

	mat4 M() {
		return mat4(1, 0, 0, 0,
			0, 1, 0, 0,
			0, 0, 1, 0,
			0, 0, 0, 1);
	}

	void UpdateVBO() {
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * points.size(), points.data(), GL_STATIC_DRAW);
	}

	void Draw() {
		UpdateVBO();
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, 1.0f, 1.0f, 1.0f);
		gpuProgram.setUniform(M(), "MVP");
		glBindVertexArray(vao);
		glDrawArrays(GL_LINE_STRIP, 0, points.size());
	}
};


class Circle {
public:
	unsigned int vao;
	unsigned int vbo;
	vec2 pos;
	float radius;
	vec3 colour;

	Circle(vec2 p, vec3 c, float r) {
		this->pos = p;
		this->colour = c;
		this->radius = r;
	}

	void Create() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);

		UpdateVertices();

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, NULL);

		gpuProgram.create(vertexSource, fragmentSource, "outColor");
	}

	void UpdateVertices() {
		vec2 vertices[nv];
		for (int i = 0; i < nv; i++) {
			float fi = i * 2 * M_PI / nv;
			float x = (radius * cosf(fi)) / (1 + sqrtf(1 - radius * radius));
			float y = (radius * sinf(fi)) / (1 + sqrtf(1 - radius * radius));
			vertices[i] = vec2(x, y);
		}
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	}

	void SetRadius(float r) {

		float newRadius = radius * expf(r);
		radius = newRadius;
		UpdateVertices();
		Create();
		Draw();
	}

	vec2 GetPos() {
		return pos;
	}

	void Draw() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, colour.x, colour.y, colour.z);


		float MVPtransf[4][4] = { 1, 0, 0, 0,
								  0, 1, 0, 0,
								  0, 0, 1, 0,
								  pos.x, pos.y, 0, 1 };

		location = glGetUniformLocation(gpuProgram.getId(), "MVP");
		glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nv);
	}

	void Move(float x, float y) {
		pos.x += x;
		pos.y += y;
	}
};

class Body : public Circle {
public:
	Body(vec2 p, vec3 c, float r): Circle( p,  c,  r){}
	void Draw() {
		int location = glGetUniformLocation(gpuProgram.getId(), "color");
		glUniform3f(location, colour.x, colour.y, colour.z);

		vec2 hyperbolic = (2.0 * pos) / (1.0 + dot(pos, pos));


		float MVPtransf[4][4] = { 1, 0, 0, 0,
								  0, 1, 0, 0,
								  0, 0, 1, 0,
								  hyperbolic.x, hyperbolic.y, 0, 1 };

		location = glGetUniformLocation(gpuProgram.getId(), "MVP");
		glUniformMatrix4fv(location, 1, GL_TRUE, &MVPtransf[0][0]);

		glBindVertexArray(vao);
		glDrawArrays(GL_TRIANGLE_FAN, 0, nv);
	}

};

Line* ufoline;
Line* foodline;
bool mouth_radius;

class UFOFood {

public:
	vec2 position;
	vec3 colour;

	Body* body;
	Circle* eye1;
	Circle* eye2;
	Circle* mouth;
	Circle* iris1;
	Circle* iris2;

	float rad;


	vec2 direction;

	float speed = 0.1f;

	UFOFood(vec2 pos, vec3 cl, float r) {
		this->position = pos;
		this->colour = cl;
		this->rad = r;
		this->body = new Body(vec2(pos.x, pos.y), cl, r);
		this->eye1 = new Circle(AttachEye(body, 0.03f, 0.20f), vec3(1.0f, 1.0f, 1.0f), 0.03f);
		this->eye2 = new Circle(AttachEye(body, 0.03f, 1.10f), vec3(1.0f, 1.0f, 1.0f), 0.03f);
		this->iris1 = new Circle(AttachIris(eye1, 0.01f, 1.10f), vec3(0.0f, 0.0f, 0.0f), 0.01f);
		this->iris2 = new Circle(AttachIris(eye2, 0.01f, 1.10f), vec3(0.0f, 0.0f, 0.0f), 0.01f);
		this->mouth = new Circle(AttachMouth(body, 0.02f, 0.65f), vec3(0.0f, 0.0f, 0.0f), 0.02f);
		this->direction = vec2(0.05f, 0.05f);
	}

	float a = atan2f(direction.y, direction.x);

	vec2 AttachEye(Body* body, float r, float angle) {
		vec2 hyperbolic = (2.0 * body->pos) / (1.0 + dot(body->pos, body->pos));
		return vec2((hyperbolic.x + (body->radius + r) * cosf(angle + a)), (hyperbolic.y + (body->radius + r) * sinf(angle + a)));
	}

	vec2 AttachMouth(Body* body, float r, float angle) {
		vec2 hyperbolic = (2.0 * body->pos) / (1.0 + dot(body->pos, body->pos));
		return vec2((hyperbolic.x + (body->radius) * cosf(angle + a)), (hyperbolic.y + (body->radius) * sinf(angle + a)));
	}

	vec2 AttachIris(Circle* eye, float r, float angle) {
		vec2 hyperbolic = (2.0 * eye->pos) / (1.0 + dot(eye->pos, eye->pos));
		return vec2((hyperbolic.x + (eye->radius - r) * cosf(angle)), (hyperbolic.y + (eye->radius - r) * sinf(angle)));
	}


	void IrisPosition(UFOFood* food) {
		vec2 directionVector1(food->position.x - this->eye1->pos.x, food->position.y - this->eye1->pos.y);
		directionVector1 = eye1->pos + directionVector1 / length(directionVector1) * (eye1->radius - iris1->radius);
		iris1->pos = directionVector1;

		vec2 directionVector2(food->position.x - this->eye2->pos.x, food->position.y - this->eye2->pos.y);
		directionVector2 = eye2->pos + directionVector2 / length(directionVector2) * (eye2->radius - iris2->radius);
		iris2->pos = directionVector2;
	}

	void Create() {
		body->Create();
		eye1->Create();
		eye2->Create();
		mouth->Create();
		iris1->Create();
		iris2->Create();
	}

	void Draw() {
		body->Draw();
		eye1->Draw();
		eye2->Draw();
		mouth->Draw();
		iris1->Draw();
		iris2->Draw();
	}

	void Move(float x, float y) {
		body->Move(x, y);
		eye1->Move(x, y);
		eye2->Move(x, y);
		mouth->Move(x, y);
		iris1->Move(x, y);
		iris2->Move(x, y);
		this->position = body->pos;
	}

	void UpdateFace(float angle) {
		eye1->pos = AttachEye(body, 0.03f, angle - 0.45f);
		eye2->pos = AttachEye(body, 0.03f, angle + 0.45f);
		iris1->pos = AttachIris(eye1, 0.01f, angle);
		iris2->pos = AttachIris(eye2, 0.01f, angle);
		mouth->pos = AttachMouth(body, 0.02f, angle);
	}


	void CircularMovement() {
		float angle = atan2f(direction.y, direction.x);
		angle -= speed;
		direction.x = cosf(angle) / (length(direction) * 1000);
		direction.y = sinf(angle) / (length(direction) * 1000);
		foodline->points.push_back(body->pos);
		this->Move(direction.x, direction.y);
		this->UpdateFace(angle);
	}

	void MouthMovement() {
		float r = mouth->radius;
		if (r > 0.025f) mouth_radius = true;

		if (r < 0.005f) mouth_radius = false;

		if (mouth_radius) mouth->SetRadius(-0.00005f);
		else mouth->SetRadius(0.00005f);
	}
};

class UFO : public UFOFood {
public:
	UFO(vec2 pos, vec3 cl, float r) : UFOFood(pos, cl, r) {}

	void MoveForward() {
		float l = length(direction);
		float x = (direction.x * speed) / (l * 50);
		float y = (direction.y * speed) / (l * 50);
		float newX = (body->pos.x + x) / (1 + body->pos.x * x + body->pos.y * y);
		float newY = (body->pos.y + y) / (1 + body->pos.x * x + body->pos.y * y);
		ufoline->points.push_back(vec2(newX, newY));
		float angle = atan2f(direction.y, direction.x);
		this->UpdateFace(angle);
		this->Move(newX - body->pos.x, newY - body->pos.y);
	}

	void Right() {
		float angle = atan2f(direction.y, direction.x);
		angle -= speed;
		direction.x = cosf(angle) / (length(direction) * 500);
		direction.y = sinf(angle) / (length(direction) * 500);
		this->UpdateFace(angle);
	}

	void Left() {
		float angle = atan2f(direction.y, direction.x);
		angle += speed;
		direction.x = cosf(angle) / (length(direction) * 500);
		direction.y = sinf(angle) / (length(direction) * 500);
		this->UpdateFace(angle);
	}
};


UFO* ufo;
UFOFood* food;
bool forward = false;

Circle* background;


void onInitialization() {
	glViewport(0, 0, windowWidth, windowHeight);

	background = new Circle(vec2(0.0f, 0.0f), vec3(0.0f, 0.0f, 0.0f), 1.0f);
	background->Create();

	ufo = new UFO(vec2(0.0f, 0.0f), vec3(1.0f, 0.0f, 0.0f), 0.1f);
	ufo->Create();
	ufoline = new Line(ufo->position);
	ufoline->Create();
	food = new UFOFood(vec2(-0.2f, -0.1f), vec3(0.0f, 1.0f, 0.0f), 0.08f);
	food->Create();
	foodline = new Line(food->position);
	foodline->Create();

	gpuProgram.create(vertexSource, fragmentSource, "outColor");
}

void onDisplay() {
	glClearColor(0.5f, 0.5f, 0.5f, 0);
	glClear(GL_COLOR_BUFFER_BIT);

	background->Draw();
	ufoline->Draw();
	ufo->Draw();
	foodline->Draw();
	food->Draw();

	glutSwapBuffers(); 

}


void onKeyboard(unsigned char key, int pX, int pY) {

	if (key == 'e') {
		forward = true;
		ufo->MoveForward();
		glutPostRedisplay();
	}

	if (key == 's') {
		ufo->Right();
		glutPostRedisplay();
	}

	if (key == 'f') {
		ufo->Left();
		glutPostRedisplay();
	}
}


void onKeyboardUp(unsigned char key, int pX, int pY) {}


void onMouseMotion(int pX, int pY) {	
	
	float cX = 2.0f * pX / windowWidth - 1;
	float cY = 1.0f - 2.0f * pY / windowHeight;
	printf("Mouse moved to (%3.2f, %3.2f)\n", cX, cY);
}


void onMouse(int button, int state, int pX, int pY) {}

long prevTime = 0;
long mouthTime = 0;
int onIdleHFPortal = 0;

void onIdle() {
	long time = glutGet(GLUT_ELAPSED_TIME); 

	
	long delta = time - prevTime;
	if (delta >= 50) {
		prevTime = time;
		food->CircularMovement();
		glutPostRedisplay();
	}

	if (forward) {
		ufo->MoveForward();
		glutPostRedisplay();
	}
	food->IrisPosition(ufo);
	ufo->IrisPosition(food);
	food->MouthMovement();
	ufo->MouthMovement();

	if (onIdleHFPortal < 3) {
		for (int i = 0; i < 40; i++) {
			food->CircularMovement();
			glutPostRedisplay();
			if (forward) {
				ufo->MoveForward();
				glutPostRedisplay();
			}
			ufo->MoveForward();
			food->IrisPosition(ufo);
			ufo->IrisPosition(food);
			food->MouthMovement();
			ufo->MouthMovement();
		}
		onIdleHFPortal++;
	}	
}
