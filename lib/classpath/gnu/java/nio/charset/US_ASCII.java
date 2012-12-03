/* US_ASCII.java -- 
   Copyright (C) 2002, 2004, 2005 Free Software Foundation, Inc.

This file is part of GNU Classpath.

GNU Classpath is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

GNU Classpath is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with GNU Classpath; see the file COPYING.  If not, write to the
Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
02110-1301 USA.

Linking this library statically or dynamically with other modules is
making a combined work based on this library.  Thus, the terms and
conditions of the GNU General Public License cover the whole
combination.

As a special exception, the copyright holders of this library give you
permission to link this library with independent modules to produce an
executable, regardless of the license terms of these independent
modules, and to copy and distribute the resulting executable under
terms of your choice, provided that you also meet, for each linked
independent module, the terms and conditions of the license of that
module.  An independent module is a module which is not derived from
or based on this library.  If you modify this library, you may extend
this exception to your version of the library, but you are not
obligated to do so.  If you do not wish to do so, delete this
exception statement from your version. */

package gnu.java.nio.charset;

import java.nio.ByteBuffer;
import java.nio.CharBuffer;
import java.nio.charset.Charset;
import java.nio.charset.CharsetDecoder;
import java.nio.charset.CharsetEncoder;
import java.nio.charset.CoderResult;

/**
 * US-ASCII charset.
 *
 * @author Jesse Rosenstock
 * @modified Ian Rogers
 */
final class US_ASCII extends Charset
{
  US_ASCII ()
  {
    /* Canonical charset name chosen according to:
     * http://java.sun.com/j2se/1.5.0/docs/guide/intl/encoding.doc.html
     */
    super ("US-ASCII", new String[] {
        /* These names are provided by 
         * http://www.iana.org/assignments/character-sets
         */
        "iso-ir-6",
        "ANSI_X3.4-1986",
        "ISO_646.irv:1991",
        "ASCII",
        "ISO646-US",
        "ASCII",
        "us",
        "IBM367",
        "cp367",
        "csASCII",
        /* These names are provided by
         * http://oss.software.ibm.com/cgi-bin/icu/convexp?s=ALL
         */
        "ANSI_X3.4-1968", "iso_646.irv:1983", "ascii7", "646",
        "windows-20127"
        });
  }

  public boolean contains (Charset cs)
  {
    return cs instanceof US_ASCII;
  }

  public CharsetDecoder newDecoder ()
  {
    return new Decoder (this);
  }

  public CharsetEncoder newEncoder ()
  {
    return new Encoder (this);
  }

  private static final class Decoder extends CharsetDecoder
  {
    /** Helper to decode loops */
    private static final ByteDecodeLoopHelper helper = new ByteDecodeLoopHelper()
    {
      protected boolean isMappable(byte b)
      {
        return b >= 0;
      }
      protected char mapToChar(byte b)
      {
        return (char)b;
      }
    };
    
    // Package-private to avoid a trampoline constructor.
    Decoder (Charset cs)
    {
      super (cs, 1.0f, 1.0f);
    }

    protected CoderResult decodeLoop (ByteBuffer in, CharBuffer out)
    {
      return helper.decodeLoop(in, out);
    }
  }

  private static final class Encoder extends CharsetEncoder
  {
    /** Helper to encode loops */
    private static final ByteEncodeLoopHelper helper = new ByteEncodeLoopHelper()
    {
      protected boolean isMappable(char c)
      {
        return c <= 0x7f;
      }
      protected byte mapToByte(char c)
      {
        return (byte)c;
      }
    };
    // Package-private to avoid a trampoline constructor.
    Encoder (Charset cs)
    {
      super (cs, 1.0f, 1.0f);
    }

    public boolean canEncode(char c)
    {
      return c <= 0x7f;
    }

    public boolean canEncode(CharSequence cs)
    {
      for (int i = 0; i < cs.length(); ++i)
        if (! canEncode(cs.charAt(i)))
          return false;
      return true;
    }

    protected CoderResult encodeLoop (CharBuffer in, ByteBuffer out)
    {
      return helper.encodeLoop(in, out);
    }
  }
}
