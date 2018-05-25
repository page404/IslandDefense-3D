//
// Created by wilmot_g on 24/03/18.
//

#include <iomanip>
#include "helpers/Glut.hpp"

#include "includes/Cannon.hpp"
#include "includes/Game.hpp"

const float g = -9.8f;

Cannon::Cannon(float speed, float radius, Color color) : _color(color),
                                                         _speed(speed),
                                                         _radius(radius),
                                                         _rotation(0),
                                                         _lastFire(-1.0f),
                                                         _lastDefence(-5.0f) {
  Vertices vertices;

  std::vector<Vertex::Ptr> top;
  std::vector<Vertex::Ptr> middle;
  std::vector<Vertex::Ptr> bottom;
  Vector3f p;
  for (int j = 0; j < 20; j++) {
    p.y = static_cast<float>(radius * std::cos(j * (360.0f / 20.0f) * M_PI / 180.0f));
    p.x = 0.0f;
    p.z = static_cast<float>(radius * std::sin(j * (360.0f / 20.0f) * M_PI / 180.0f));
    bottom.push_back(std::make_shared<Vertex>(p));
    p.x = _radius * 5.0f;
    middle.push_back(std::make_shared<Vertex>(p));
    p.x = _radius * 10.0f;
    top.push_back(std::make_shared<Vertex>(p));
  }
  vertices.push_back(bottom);
  vertices.push_back(middle);
  vertices.push_back(top);

  Triangles triangles;
  Vertex::Ptr centerBottom = std::make_shared<Vertex>(Vector3f(0.0f, 0.0f, 0.0f));
  Vertex::Ptr centerTop = std::make_shared<Vertex>(Vector3f(_radius * 10.0f, 0.0f, 0.0f));
  Vertex::Ptr bl = vertices[0][vertices[0].size() - 1];
  Vertex::Ptr br = vertices[0][0];
  Vertex::Ptr ml = vertices[1][vertices[1].size() - 1];
  Vertex::Ptr mr = vertices[1][0];
  Vertex::Ptr tl = vertices[2][vertices[2].size() - 1];
  Vertex::Ptr tr = vertices[2][0];
  triangles.emplace_back(bl, ml, mr, Triangle::computeNormal(bl->p, ml->p, mr->p));
  triangles.emplace_back(mr, br, bl, Triangle::computeNormal(mr->p, br->p, bl->p));
  triangles.emplace_back(ml, tl, tr, Triangle::computeNormal(ml->p, tl->p, tr->p));
  triangles.emplace_back(tr, mr, ml, Triangle::computeNormal(tr->p, mr->p, ml->p));
  triangles.emplace_back(br, centerBottom, bl, Triangle::computeNormal(br->p, centerBottom->p, bl->p));
  triangles.emplace_back(tl, centerTop, tr, Triangle::computeNormal(tl->p, centerTop->p, tr->p));
  for (int i = 0; i < vertices[0].size() - 1; i++) {
    bl = vertices[0][i];
    br = vertices[0][i + 1];
    ml = vertices[1][i];
    mr = vertices[1][i + 1];
    tl = vertices[2][i];
    tr = vertices[2][i + 1];
    triangles.emplace_back(bl, ml, mr, Triangle::computeNormal(bl->p, ml->p, mr->p));
    triangles.emplace_back(mr, br, bl, Triangle::computeNormal(mr->p, br->p, bl->p));
    triangles.emplace_back(ml, tl, tr, Triangle::computeNormal(ml->p, tl->p, tr->p));
    triangles.emplace_back(tr, mr, ml, Triangle::computeNormal(tr->p, mr->p, ml->p));
    triangles.emplace_back(br, centerBottom, bl, Triangle::computeNormal(br->p, centerBottom->p, bl->p));
    triangles.emplace_back(tl, centerTop, tr, Triangle::computeNormal(tl->p, centerTop->p, tr->p));
  }
  Shape shape = Shape(triangles, GL_TRIANGLES, color);
  shape.computePerVertexNormal();
  _shapes.push_back(shape);
}

void Cannon::drawTrajectory() const {
  glPushMatrix();

  GLfloat rotation1[16], rotation2[16], translation[16], first[16], final[16];
  _coordinates.toTranslationMatrix(translation);
  (_angle * (M_PI / 180.0f)).toRotationMatrix(rotation1);
  (Vector3f{0.0f, 0.0f, _rotation} * (M_PI / 180.0f)).toRotationMatrix(rotation2);
  Vector3f::multMatrix(translation, rotation1, first);
  Vector3f::multMatrix(first, rotation2, final);

  Vector3f c = Vector3f(_radius * 10.0f, 0.0f, 0.0f) * final;

  glBegin(GL_LINE_STRIP);
  float t = 0;
  for (;;) {
    float x = c.x + _velocity.x * t;
    float y = c.y + _velocity.y * t + g * t * t / 2.0f;
    float z = c.z + _velocity.z * t;

    if (y < -1 || y > 1 || x < -1 || x > 1 || z < -1 || z > 1) {
      break;
    }

    glVertex3f(x, y, z);
    t += 0.01;
  }
  glEnd();
  glPopMatrix();
}

void Cannon::draw() const {
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_BLEND);
  glEnable(GL_COLOR_MATERIAL);
  glEnable(GL_NORMALIZE);

  GLfloat specular[] = {1.0f, 0.3f, 0.5f, 1.0f};
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, specular);
  GLfloat diffuse[] = {0.5f, 0.5f, 0.5f, 1.0f};
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, diffuse);
  GLfloat emission[] = {0.0f, 0.0f, 0.0f, 1.0f};
  glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, emission);
  GLfloat shininess = 64.0f;
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, shininess);

  glPushMatrix();

  GLfloat m[16];
  glMultMatrixf(_coordinates.toTranslationMatrix(m));
  glMultMatrixf((_angle * (M_PI / 180.0f)).toRotationMatrix(m));
  glMultMatrixf((Vector3f{0.0f, 0.0f, _rotation} * (M_PI / 180.0f)).toRotationMatrix(m));

  Displayable::draw();
  _shapes.front().applyColor();
  glutSolidSphere(_radius * 2.0f, 20, 20);

  glPopMatrix();

  glDisable(GL_NORMALIZE);
  glDisable(GL_COLOR_MATERIAL);
  glDisable(GL_BLEND);
  glDisable(GL_LIGHT0);
  glDisable(GL_LIGHTING);

  drawTrajectory();

  _projectiles.draw();
  _defences.draw();
}

void Cannon::blast() {
  if (Game::getInstance().getTime() - _lastFire > 1.0f) {
    _lastFire = Game::getInstance().getTime();
    float rotation[16];
    Vector3f(_angle * (M_PI / 180.0f)).toRotationMatrix(rotation);
    _projectiles.add(std::make_shared<Projectile>(Game::getInstance().getTime(),
                                                  (Vector3f{_radius * 10.0f, 0, 0} * rotation) + _coordinates,
                                                  _velocity * rotation,
                                                  _color));
  }
}

void Cannon::defend() {
  if (Game::getInstance().getTime() - _lastDefence > 5.0f) {
    _lastDefence = Game::getInstance().getTime();
    _defences.add(std::make_shared<Pellet>(Game::getInstance().getTime(), Vector3f(), _color));
  }
}

void Cannon::speed(float value) {
  _speed += value;
  _speed = _speed < 0 ? 0 : _speed;
}

void Cannon::rotation(float angle) {
  _rotation += angle;
  _rotation = _rotation < 0 ? 0 : _rotation;
  _rotation = _rotation > 180 ? 180 : _rotation;
}

void Cannon::setPos(Vector3f coordinates, Vector3f angle) {
  _coordinates = coordinates;
  _angle = angle;
}

void Cannon::update() {
  _velocity.x = static_cast<float>(std::cos((_rotation + _angle.z) * M_PI / 180.0f) * _speed);
  _velocity.y = static_cast<float>(std::sin((_rotation + _angle.z) * M_PI / 180.0f) * _speed);
//  std::cout << "velocity : " << _velocity << std::endl;
  _velocity.z = static_cast<float>(std::sin(_angle.y * M_PI / 180.0f) * _speed);
  _projectiles.update();
  _defences.update();
}