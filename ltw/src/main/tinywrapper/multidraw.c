/**
 * Created by: artDev
 * Copyright (c) 2025 artDev, SerpentSpirale, CADIndie.
 * For use under LGPL-3.0
 */

#include <proc.h>
#include <egl.h>
#include "basevertex.h"
typedef struct {
    GLuint count;
    GLuint instanceCount;
    GLuint first;
    GLuint reserved;
} DrawArraysIndirectCommand;

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
    GLint elementbuffer;
    es3_functions.glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING, &elementbuffer);
    es3_functions.glBindBuffer(GL_COPY_WRITE_BUFFER, current_context->multidraw_element_buffer);
    GLsizei total = 0, offset = 0, typebytes = type_bytes(type);
    for (GLsizei i = 0; i < primcount; i++) {
        total += count[i];
    }
    es3_functions.glBufferData(GL_COPY_WRITE_BUFFER, total*typebytes, NULL, GL_STREAM_DRAW);
    for (GLsizei i = 0; i < primcount; i++) {
        GLsizei icount = count[i];
        if(icount == 0) continue;
        icount *= typebytes;
        if(elementbuffer != 0) {
            es3_functions.glCopyBufferSubData(GL_ELEMENT_ARRAY_BUFFER, GL_COPY_WRITE_BUFFER, (GLintptr)indices[i], offset, icount);
        }else {
            es3_functions.glBufferSubData(GL_COPY_WRITE_BUFFER, offset, icount, indices[i]);
        }
        offset += icount;
    }
    es3_functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current_context->multidraw_element_buffer);
    es3_functions.glDrawElements(mode, total, type, 0);
    if(elementbuffer != 0) es3_functions.glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementbuffer);

}