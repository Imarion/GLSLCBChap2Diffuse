#include "Diffuse.h"

#include <QtGlobal>

#include <QDebug>
#include <QFile>
#include <QImage>
#include <QTime>

#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>

#include <cmath>
#include <cstring>

MyWindow::~MyWindow()
{
    if (mProgram  != 0)   delete   mProgram;
}

MyWindow::MyWindow() : currentTimeMs(0), currentTimeS(0)
{
    mProgram  = 0;

    setSurfaceType(QWindow::OpenGLSurface);
    setFlags(Qt::Window | Qt::WindowSystemMenuHint | Qt::WindowTitleHint | Qt::WindowMinMaxButtonsHint | Qt::WindowCloseButtonHint);

    QSurfaceFormat format;
    format.setDepthBufferSize(24);
    format.setMajorVersion(4);
    format.setMinorVersion(3);
    format.setSamples(4);
    format.setProfile(QSurfaceFormat::CoreProfile);
    setFormat(format);
    create();

    resize(800, 600);

    mContext = new QOpenGLContext(this);
    mContext->setFormat(format);
    mContext->create();

    mContext->makeCurrent( this );

    mFuncs = mContext->versionFunctions<QOpenGLFunctions_4_3_Core>();
    if ( !mFuncs )
    {
        qWarning( "Could not obtain OpenGL versions object" );
        exit( 1 );
    }
    if (mFuncs->initializeOpenGLFunctions() == GL_FALSE)
    {
        qWarning( "Could not initialize core open GL functions" );
        exit( 1 );
    }

    initializeOpenGLFunctions();

    QTimer *repaintTimer = new QTimer(this);
    connect(repaintTimer, &QTimer::timeout, this, &MyWindow::render);
    repaintTimer->start(1000/60);

    QTimer *elapsedTimer = new QTimer(this);
    connect(elapsedTimer, &QTimer::timeout, this, &MyWindow::modCurTime);
    elapsedTimer->start(1);       
}

void MyWindow::modCurTime()
{
    currentTimeMs++;
    currentTimeS=currentTimeMs/1000.0f;
}

void MyWindow::initialize()
{
    mFuncs->glGenVertexArrays(1, &mVAO);
    mFuncs->glBindVertexArray(mVAO);

    CreateVertexBuffer();
    initShaders();

    mRotationMatrixLocation = mProgram->uniformLocation("RotationMatrix");

    glFrontFace(GL_CCW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
}

void MyWindow::CreateVertexBuffer()
{
     mTorus = new Torus(0.7f, 0.3f, 30, 30);

     // Create and populate the buffer objects
     unsigned int handle[4];
     glGenBuffers(4, handle);

     glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
     glBufferData(GL_ARRAY_BUFFER, (3 * mTorus->getnVerts()) * sizeof(float), mTorus->getv(), GL_STATIC_DRAW);

     glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
     glBufferData(GL_ARRAY_BUFFER, (3 * mTorus->getnVerts()) * sizeof(float), mTorus->getn(), GL_STATIC_DRAW);

     glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
     glBufferData(GL_ARRAY_BUFFER, (2 * mTorus->getnVerts()) * sizeof(float), mTorus->gettex(), GL_STATIC_DRAW);

     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[3]);
     glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * mTorus->getnFaces() * sizeof(unsigned int), mTorus->getel(), GL_STATIC_DRAW);

     // Create the VAO
     glEnableVertexAttribArray(0);  // Vertex position
     glBindBuffer(GL_ARRAY_BUFFER, handle[0]);
     glVertexAttribPointer( (GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

     glEnableVertexAttribArray(1);  // Vertex normal
     glBindBuffer(GL_ARRAY_BUFFER, handle[1]);
     glVertexAttribPointer( (GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

     glBindBuffer(GL_ARRAY_BUFFER, handle[2]);
     glEnableVertexAttribArray(2);  // Texture coords
     glVertexAttribPointer( (GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, ((GLubyte *)NULL + (0)) );

     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle[3]);

     mFuncs->glBindVertexArray(0);


/*
    float positionData[] = {
        -0.8f, -0.8f, 0.0f,
         0.8f, -0.8f, 0.0f,
         0.0f,  0.8f, 0.0f
    };

    float colorData[]= {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f,
    };

    GLuint vboHandles[2];

    glGenBuffers(2, vboHandles);
    mPositionBufferHandle = vboHandles[0];
    mColorBufferHandle    = vboHandles[1];

    glBindBuffer(GL_ARRAY_BUFFER, mPositionBufferHandle);
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float), positionData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mColorBufferHandle);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 9 * sizeof(float), colorData, GL_STATIC_DRAW);
*/
}

void MyWindow::resizeEvent(QResizeEvent *)
{
    mUpdateSize = true;
}

void MyWindow::render()
{
    if(!isVisible() || !isExposed())
        return;

    if (!mContext->makeCurrent(this))
        return;

    static bool initialized = false;
    if (!initialized) {
        initialize();
        initialized = true;
    }

    if (mUpdateSize) {
        glViewport(0, 0, size().width(), size().height());
        mUpdateSize = false;
    }

    static float EvolvingVal = 0;
    EvolvingVal += 0.1f;

    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QMatrix4x4 RotationMatrix;

    RotationMatrix.rotate(EvolvingVal * 3.0f, QVector3D(0.1f, 0.0f, 0.1f));

    mFuncs->glBindVertexArray(mVAO);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
/*
    mFuncs->glBindVertexBuffer(0, mPositionBufferHandle, 0, sizeof(GLfloat) * 3);
    mFuncs->glBindVertexBuffer(1, mColorBufferHandle,    0, sizeof(GLfloat) * 3);

    mFuncs->glVertexAttribFormat(0, 3, GL_FLOAT, GL_FALSE, 0);
    mFuncs->glVertexAttribBinding(0, 0);

    mFuncs->glVertexAttribFormat(1, 3, GL_FLOAT, GL_FALSE, 0);
    mFuncs->glVertexAttribBinding(1, 1);
*/
    mProgram->bind();
    {
        glUniformMatrix4fv(mRotationMatrixLocation, 1, GL_FALSE, RotationMatrix.constData());
        //glDrawArrays(GL_TRIANGLES, 0, 3);
        glDrawElements(GL_TRIANGLES, 6 * mTorus->getnFaces(), GL_UNSIGNED_INT, ((GLubyte *)NULL + (0)));

        glDisableVertexAttribArray(0);
        glDisableVertexAttribArray(1);
    }
    mProgram->release();

    mContext->swapBuffers(this);
}

void MyWindow::initShaders()
{
    QOpenGLShader vShader(QOpenGLShader::Vertex);
    QOpenGLShader fShader(QOpenGLShader::Fragment);    
    QFile         shaderFile;
    QByteArray    shaderSource;

    //mTreeProgram
    // Shader 1
    shaderFile.setFileName(":/vshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "vertex tree compile: " << vShader.compileSourceCode(shaderSource);

    shaderFile.setFileName(":/fshader.txt");
    shaderFile.open(QIODevice::ReadOnly);
    shaderSource = shaderFile.readAll();
    shaderFile.close();
    qDebug() << "frag   tree compile: " << fShader.compileSourceCode(shaderSource);

    mProgram = new (QOpenGLShaderProgram);
    mProgram->addShader(&vShader);
    mProgram->addShader(&fShader);
    qDebug() << "shader link tree: " << mProgram->link();
}

void MyWindow::PrepareTexture(GLenum TextureTarget, const QString& FileName, GLuint& TexObject, bool flip)
{
    QImage TexImg;

    if (!TexImg.load(FileName)) qDebug() << "Erreur chargement texture";
    if (flip==true) TexImg=TexImg.mirrored();

    glGenTextures(1, &TexObject);
    glBindTexture(TextureTarget, TexObject);
    glTexImage2D(TextureTarget, 0, GL_RGB, TexImg.width(), TexImg.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE, TexImg.bits());
    glTexParameterf(TextureTarget, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameterf(TextureTarget, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void MyWindow::keyPressEvent(QKeyEvent *keyEvent)
{
    switch(keyEvent->key())
    {
        case Qt::Key_P:
            break;
        case Qt::Key_Up:
            break;
        case Qt::Key_Down:
            break;
        case Qt::Key_Left:
            break;
        case Qt::Key_Right:
            break;
        case Qt::Key_Delete:
            break;
        case Qt::Key_PageDown:
            break;
        case Qt::Key_Home:
            break;
        case Qt::Key_Z:
            break;
        case Qt::Key_Q:
            break;
        case Qt::Key_S:
            break;
        case Qt::Key_D:
            break;
        case Qt::Key_A:
            break;
        case Qt::Key_E:
            break;
        default:
            break;
    }
}


void MyWindow::printMatrix(const QMatrix4x4& mat)
{
    const float *locMat = mat.transposed().constData();

    for (int i=0; i<4; i++)
    {
        qDebug() << locMat[i*4] << " " << locMat[i*4+1] << " " << locMat[i*4+2] << " " << locMat[i*4+3];
    }
}
