/**
 * Created by: artDev
 * Copyright (c) 2025 artDev, SerpentSpirale, CADIndie.
 * For use under LGPL-3.0
 */

#include <proc.h>
#include <egl.h>
#include "basevertex.h"
#include "main.h"
typedef struct {
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint reserved;
} DrawArraysIndirectCommand;

typedef struct {
    GLuint count;
    GLuint instanceCount;
    GLuint firstIndex;
    GLint  baseVertex;
    GLuint reserved;
} DrawElementsIndirectCommand;

void glMultiDrawArrays( GLenum mode, GLint *first, GLsizei *count, GLsizei primcount )
{
    if(!current_context) return;
    if (current_context->es31 && primcount > 8) {
        DrawArraysIndirectCommand commands[primcount];
        for (int i = 0; i < primcount; i++) {
            commands[i].count = count[i];
            commands[i].instanceCount = 1;
            commands[i].first = first[i];
            commands[i].reserved = 0;
        }
        GLuint prev_buffer = current_context->bound_buffers[get_buffer_index(GL_DRAW_INDIRECT_BUFFER)];
        es3_functions.glBindBuffer(GL_DRAW_INDIRECT_BUFFER, current_context->multidraw_element_buffer);
        es3_functions.glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(commands), commands, GL_STREAM_DRAW);
        for (int i = 0; i < primcount; i++) {
            es3_functions.glDrawArraysIndirect(mode, (void*)(uintptr_t)(i * sizeof(DrawArraysIndirectCommand)));
        }
        es3_functions.glBindBuffer(GL_DRAW_INDIRECT_BUFFER, prev_buffer);
        return;
    }
    for (int i = 0; i < primcount; i++) {
        if (count[i] > 0)
            es3_functions.glDrawArrays(mode, first[i], count[i]);
    }
}

void glMultiDrawElements( GLenum mode, GLsizei *count, GLenum type, const void * const *indices, GLsizei primcount )
{
    if(!current_context) return;
    
    // Performance optimization for Sodium/MC 1.20+: Use DrawElementsIndirect if available
    if (current_context->es31 && primcount > 8) {
        DrawElementsIndirectCommand commands[primcount];
        GLsizei typebytes = type_bytes(type);
        for (int i = 0; i < primcount; i++) {
            commands[i].count = count[i];
            commands[i].instanceCount = 1;
            commands[i].firstIndex = (GLuint)((uintptr_t)indices[i] / typebytes);
            commands[i].baseVertex = 0;
            commands[i].reserved = 0;
        }
        
        GLuint prev_indirect = current_context->bound_buffers[get_buffer_index(GL_DRAW_INDIRECT_BUFFER)];
        // Reuse the multidraw buffer for commands
        es3_functions.glBindBuffer(GL_DRAW_INDIRECT_BUFFER, current_context->multidraw_element_buffer);
        es3_functions.glBufferData(GL_DRAW_INDIRECT_BUFFER, sizeof(commands), commands, GL_STREAM_DRAW);
        
        for (int i = 0; i < primcount; i++) {
            es3_functions.glDrawElementsIndirect(mode, type, (void*)(uintptr_t)(i * sizeof(DrawElementsIndirectCommand)));
        }
        
        es3_functions.glBindBuffer(GL_DRAW_INDIRECT_BUFFER, prev_indirect);
        return;
    }

    // Fallback for older GLES or small draw counts
    for (int i = 0; i < primcount; i++) {
        if (count[i] > 0)
            es3_functions.glDrawElements(mode, count[i], type, indices[i]);
    }
}