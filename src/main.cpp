#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <cmath>

static const char recipe[] =
    "lidjehfhfhhideiefedefedefekedeiefedefedefejfdeiefedefedefejeeieefed"
    "efedefeiekedefedefedefeiekedefedefedefeiefefedefedefedefeieghfhfhfhm";

int main() {
	sf::RenderWindow window(sf::VideoMode(1920, 1080), "test",
							sf::Style::Default, sf::ContextSettings(24,0,2));
	window.setVerticalSyncEnabled(true);

	window.resetGLStates();
	glDisableClientState(GL_NORMAL_ARRAY);
	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.f);
	
	sf::Texture tx[7];
	tx[0].loadFromFile("resources/bottom.jpg");
    tx[1].loadFromFile("resources/top.jpg");
    tx[2].loadFromFile("resources/left.jpg");
    tx[3].loadFromFile("resources/right.jpg");
    tx[4].loadFromFile("resources/back.jpg");
    tx[5].loadFromFile("resources/front.jpg");
    tx[6].loadFromFile("resources/wall3.jpg");
    tx[6].generateMipmap();


	std::vector<GLfloat> tri;
    auto addcuboid = [&](unsigned mask,
                         std::array<float,2> x, std::array<float,2> z, std::array<float,2> y,
                         std::array<float,3> c, std::array<float,3> u, std::array<float,3> v)
    {
        auto ext = [](auto m,unsigned n,unsigned b=1) { return (m >> (n*b)) & ~(~0u << b); }; // extracts bits
        // Generates: For six vertices, color(rgb), coordinate(xyz) and texture coord(uv).
        std::array p{&c[0],&c[0],&c[0], &x[0],&y[0],&z[0], &u[0],&v[0]};
        // capflag(1 bit), mask(3 bits), X(4 bits), Y(4 bits), Z(4 bits), U(4 bits), V(4 bits)
        for(unsigned m: std::array{0x960339,0xA9F339,0x436039,0x4C6F39,0x406C39,0x4F6339}) // bottom, top, four sides
            if(std::uint64_t s = (m>>23) * 0b11'000'111 * (~0llu/255); mask & m)
                for(unsigned n = 0; n < 6*8; ++n)
                    tri.push_back( p[n%8][ext(m, ext(012345444u, n%8, 3)*4 - ext(0123341u, n/8, 3)) << ext(s,n)] );
        // 123341 = order of vertices in two triangles; 12345444 = nibble indexes in "m" for each of 8 values
    };



	addcuboid(7<<20, {-10,10}, {-10,10}, {-10,10}, { 1, 1, 1}, {0,1,1},  {0,1,1});

    addcuboid(1<<20, {-30,30}, {-30,30}, {0,10},   {.3,.3,.4}, {0,0,60}, {0,0,60});

    for(int rem=0,p=0,z=-14; z<15; ++z)
        for(int x=-21; x<21; ++x)
        {
            if(!rem--) { rem = recipe[p++] - 'd'; if(rem&8) rem+=414; } // RLE compression, odd/even coding
            if(float w=.5f, h = (p&1) ? (std::rand() % 2)*.05f : .8f*(4+std::rand()%8); h) // Random height
                addcuboid(6<<20, {x-w,x+w}, {z-w,z+w}, {0,h}, {.2f+(rand()%1000)*.4e-3f,1,.4f+(h>.1f)}, {0,1,1},{0,h,1});
        }
    glColorPointer(3,    GL_FLOAT, 8*sizeof(GLfloat), &tri[0]);
    glVertexPointer(3,   GL_FLOAT, 8*sizeof(GLfloat), &tri[3]);
    glTexCoordPointer(2, GL_FLOAT, 8*sizeof(GLfloat), &tri[6]);

    glMatrixMode(GL_PROJECTION);
    glViewport(0, 0, window.getSize().x, window.getSize().y);
    GLfloat near=.03f, far=50.f, ratio = near * window.getSize().x / window.getSize().y;
    glFrustum(-ratio, ratio, -near, near, near, far);

    // Start game loop
    float rx=0,ry=1,rz=-.1, mx=0,my=0,mz=-2.5, lx=3.86,ly=-0.29,lz=15.9, aa=0.92,ab=-0.18,ac=-0.35,ad=0.07, fog=4;
    GLfloat tform[16]{1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};

    for(std::map<int,bool> keys; window.isOpen() && !keys[sf::Keyboard::Escape]; window.display())
    {
        for(sf::Event event; window.pollEvent(event); )
            switch(event.type)
            {
                case sf::Event::Closed:      keys[sf::Keyboard::Escape] = true; default: break;
                case sf::Event::KeyPressed:  keys[event.key.code] = true; break;
                case sf::Event::KeyReleased: keys[event.key.code] = false; break;
            }
        if(keys[sf::Keyboard::V]) { for(std::size_t p=6*6*8; p<tri.size(); p+=8) if(tri[p+4]>0.1)tri[p+4] *= 0.95; fog *= 0.95; }


        bool up    = keys[sf::Keyboard::Up]   || keys[sf::Keyboard::Numpad8];
        bool down  = keys[sf::Keyboard::Down] || keys[sf::Keyboard::Numpad2],     alt   = keys[sf::Keyboard::LAlt]|| keys[sf::Keyboard::RAlt];
        bool left  = keys[sf::Keyboard::Left] || keys[sf::Keyboard::Numpad4],     rleft = keys[sf::Keyboard::Q]   || keys[sf::Keyboard::Numpad7];
        bool right = keys[sf::Keyboard::Right]|| keys[sf::Keyboard::Numpad6],     rright= keys[sf::Keyboard::E]   || keys[sf::Keyboard::Numpad9];
        bool fwd   = keys[sf::Keyboard::A], sup   = keys[sf::Keyboard::Subtract], sleft = keys[sf::Keyboard::Numpad1];
        bool back  = keys[sf::Keyboard::Z], sdown = keys[sf::Keyboard::Add],      sright= keys[sf::Keyboard::Numpad3];

        rx = rx*.8f + .2f*(up     - down) * !alt;
        ry = ry*.8f + .2f*(right  - left) * !alt;
        rz = rz*.8f + .2f*(rright - rleft);
        if(float rlen = std::sqrt(rx*rx + ry*ry + rz*rz); rlen > 1e-3f)
        {

            float theta = rlen*.03f, c = std::cos(theta*.5f), s = std::sin(theta*.5f)/rlen;
            auto [qa,qb,qc,qd] = std::array{ c,
                                             s*(tform[0]*rx + tform[1]*ry + tform[2]*rz),
                                             s*(tform[4]*rx + tform[5]*ry + tform[6]*rz),
                                             s*(tform[8]*rx + tform[9]*ry + tform[10]*rz) };

            std::tie(aa,ab,ac,ad) = std::tuple{ qa*aa - qb*ab - qc*ac - qd*ad,
                                                qb*aa + qa*ab + qd*ac - qc*ad,
                                                qc*aa - qd*ab + qa*ac + qb*ad,
                                                qd*aa + qc*ab - qb*ac + qa*ad };

            std::tie(aa,ab,ac,ad) = std::tuple{ aa * (1.f/std::sqrt(aa*aa+ab*ab+ac*ac+ad*ad)),
                                                ab * (1.f/std::sqrt(aa*aa+ab*ab+ac*ac+ad*ad)),
                                                ac * (1.f/std::sqrt(aa*aa+ab*ab+ac*ac+ad*ad)),
                                                ad * (1.f/std::sqrt(aa*aa+ab*ab+ac*ac+ad*ad)) };
            tform[0] = 1-2*(ac*ac+ad*ad); tform[1] =   2*(ab*ac+aa*ad); tform[2] =   2*(ab*ad-aa*ac);
            tform[4] =   2*(ab*ac-aa*ad); tform[5] = 1-2*(ab*ab+ad*ad); tform[6] =   2*(ac*ad+aa*ab);
            tform[8] =   2*(ab*ad+aa*ac); tform[9] =   2*(ac*ad-aa*ab); tform[10]= 1-2*(ab*ab+ac*ac);
        }

        float Mx = (sleft || (alt && left)) - (sright || (alt && right));
        float My = (sdown || (alt && down)) - (sup    || (alt && up));
        float Mz = fwd - back;
        float mlen = std::sqrt(Mx*Mx + My*My + Mz*Mz)/0.07; if(mlen < 1e-3f) mlen = 1;
        mx = mx*.9f + .1f*(tform[0]*Mx + tform[1]*My + tform[2]*Mz)/mlen;
        my = my*.9f + .1f*(tform[4]*Mx + tform[5]*My + tform[6]*Mz)/mlen;
        mz = mz*.9f + .1f*(tform[8]*Mx + tform[9]*My + tform[10]*Mz)/mlen;
        lx += mx; ly += my; lz += mz;


		//Fog
        glEnable(GL_FOG);
        glFogi(GL_FOG_MODE,    GL_EXP);
        glFogfv(GL_FOG_COLOR,  &std::array{.5f,.51f,.54f}[0]);
        glFogf(GL_FOG_DENSITY, fog/far);

        // Instruct OpenGL about the view rotation.
        glMatrixMode(GL_MODELVIEW);
        glLoadMatrixf(tform);


        glClear(GL_DEPTH_BUFFER_BIT);
        glDepthMask(GL_FALSE);
        for(unsigned n=0; n<6; ++n)
        {
            sf::Texture::bind(&tx[n]);
            glDrawArrays(GL_TRIANGLES, n*6, 6);
        }

        glTranslatef(lx, ly, lz);
        glDepthMask(GL_TRUE);

        sf::Texture::bind(&tx[6]);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glDrawArrays(GL_TRIANGLES, 6*6, tri.size()/8-6*6);
	}
}
