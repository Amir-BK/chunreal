// name: intqueue.ck
// desc: implements a queue that holds two elements, a "val" and a "voice"
//       addElem: with both value and voice
//       removeElem: by value, returns voice, or -1 if not found
//       removeOldestElem: returns voice, or -1 if not found
// author: Jack Atherton

class IntElem
{
    int val;
    int voice;
    null @=> IntElem @ next;
    null @=> IntElem @ prev;
}

public class IntQueue
{
    
    null @=> IntElem head;
    null @=> IntElem tail;
    0 => int numElems;
    
    fun int size()
    {
        return numElems;
    }
    
    fun int addElem( int v, int voice )
    {
        IntElem newElem;
        v => newElem.val;
        voice => newElem.voice;
        
        if( numElems == 0 )
        {
            newElem @=> head;
        }
        else
        {
            newElem @=> tail.next;
            tail @=> newElem.prev;
        }
        newElem @=> tail;
        
        return numElems++;
    }
    
    fun int removeElem( int v )
    {
        head @=> IntElem current;
        while( current != null )
        {
            if( current.val == v )
            {
                numElems--;
                current.voice => int voice;
                if( current.prev != null )
                {
                    current.next @=> current.prev.next;
                }
                if( current.next != null )
                {
                    current.prev @=> current.next.prev;
                }
                if( current == head )
                {
                    current.next @=> head;
                }
                if( current == tail )
                {
                    current.prev @=> tail;
                }
                null @=> current.prev;
                null @=> current.next;
                
                return voice;
            }
            current.next @=> current;
        }
        
        return -1;
    }
    
    fun int removeOldestElem()
    {
        if( numElems == 0 )
        {
            return -1;
        }
        else if( numElems == 1 )
        {
            head.voice => int ret;
            numElems--;
            // <<< head.val, head.voice, "auto removed" >>>;
            null @=> head;
            null @=> tail; 
            return ret;
        }
        else
        {
            head.voice => int ret;
            // <<< head.val, head.voice, "auto removed" >>>;
            head.next @=> head;
            if( head != null )
            {
                null @=> head.prev;
            }
            numElems--;
            return ret;
        }
    }
}
