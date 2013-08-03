#include "TexturedQuad.h"
#include "TextureServer.h"
#include "TextWriter.h"
#include "File.h"

namespace Amju
{
TexturedQuad::TexturedQuad() : m_pTex(0)
{
  m_areaTop = 0;
  m_areaLeft = 0;
  m_areaBottom = 1.0f;
  m_areaRight = 1.0f;
}

#ifdef SCENE_EDITOR
bool TexturedQuad::Save(File* pf)
{
  if (!pf->Write(m_pTex->GetFileName()))
  {
    pf->ReportError("Failed to save textured quad");
    return false;
  }
  if (m_pTex->GetAlphaFilename().empty())
  {
    pf->WriteInteger(0); 
  }
  else
  {
    pf->WriteInteger(1);
    pf->Write(m_pTex->GetAlphaFilename());
  }
  return true;
}
#endif

bool TexturedQuad::Load(File* pf)
{
  // Get texture filename
  std::string textureName;
  if (!pf->GetDataLine(&textureName))
  {
    pf->ReportError("Expected texture file name.");
    return false;
  }

  // Get flag: 1 means alpha, 0 means no alpha
  int flag = 0;
  if (!pf->GetInteger(&flag))
  {
    pf->ReportError("Expected alpha flag.");
    return false;
  }

  // Get alpha layer filename
  std::string alphaName;
  if (flag == 1)
  { 
    if (!pf->GetDataLine(&alphaName))
    {
      pf->ReportError("Expected alpha file name.");
      return false;    
    }
  }
  else if (flag != 0)
  {
    pf->ReportError("Bad textured quad alpha flag value.");
    return false;
  }

  // Load the textures
  return Load(textureName, alphaName);
}

bool TexturedQuad::Load(const std::string& tex, const std::string& alpha)
{
  m_pTex = TextureServer::Instance()->Get(tex, alpha);
  if (m_pTex)
  {
    m_pTex->CreateBinding(PoolTexture::REGULAR);
  }
  return (m_pTex != 0);
}

void TexturedQuad::Draw(float top, float left, float bottom, float right)
{
  if (!m_pTex)
  {
    return;
  }

  float xmax = right * TextWriter::CHAR_SIZE - TextWriter::X_OFFSET;
  float xmin = left * TextWriter::CHAR_SIZE - TextWriter::X_OFFSET;
  float ymax = TextWriter::Y_OFFSET - (top - 1) * TextWriter::CHAR_SIZE;
  float ymin = TextWriter::Y_OFFSET - (bottom - 1) * TextWriter::CHAR_SIZE;

  AmjuGL::Disable(AmjuGL::AMJU_DEPTH_READ);

  m_pTex->Bind();

  float y1 = 1.0f - m_areaBottom; // usually == 0
  float y2 = 1.0f - m_areaTop; // usually == 1.0
  AmjuGL::Vert verts[4] =
  {
    // x, y, z, u, v, nx, ny, nz
    AmjuGL::Vert(xmin, ymin, TextWriter::Z_OFFSET,   m_areaLeft,  y1,     1, 1, 0), 
    AmjuGL::Vert(xmax, ymin, TextWriter::Z_OFFSET,   m_areaRight, y1,     0, 1, 0),
    AmjuGL::Vert(xmax, ymax, TextWriter::Z_OFFSET,   m_areaRight, y2,     0, 1, 1),
    AmjuGL::Vert(xmin, ymax, TextWriter::Z_OFFSET,   m_areaLeft,  y2,     0, 1, 0)
  };

  AmjuGL::Tris tris;
  tris.reserve(2);
  AmjuGL::Tri tri;
  tri.m_verts[0] = verts[0];
  tri.m_verts[1] = verts[1];
  tri.m_verts[2] = verts[2];
  tris.push_back(tri);

  tri.m_verts[0] = verts[0];
  tri.m_verts[1] = verts[2];
  tri.m_verts[2] = verts[3];
  tris.push_back(tri);
  
  if (!m_triList)
  {
    m_triList = (TriListDynamic*)AmjuGL::Create(TriListDynamic::DRAWABLE_TYPE_ID);
  }
  m_triList->Set(tris);
  AmjuGL::Draw(m_triList);
}

void TexturedQuad::SetDrawArea(float t, float l, float b, float r)
{
  m_areaTop = t;
  m_areaLeft = l;
  m_areaBottom = b;
  m_areaRight = r;
}

}
