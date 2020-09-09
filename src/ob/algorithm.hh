/*
                                    88888888
                                  888888888888
                                 88888888888888
                                8888888888888888
                               888888888888888888
                              888888  8888  888888
                              88888    88    88888
                              888888  8888  888888
                              88888888888888888888
                              88888888888888888888
                             8888888888888888888888
                          8888888888888888888888888888
                        88888888888888888888888888888888
                              88888888888888888888
                            888888888888888888888888
                           888888  8888888888  888888
                           888     8888  8888     888
                                   888    888

                                   OCTOBANANA

Licensed under the MIT License

Copyright (c) 2019 Brett Robinson <https://octobanana.com/>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#ifndef OB_ALGORITHM_HH
#define OB_ALGORITHM_HH

#include <cstddef>

#include <functional>

namespace OB::Algorithm
{

template<class Op>
void for_each(std::size_t const cn, Op const& op)
{
  if (cn == 0) return;
  for (std::size_t i = 0; i < cn; ++i)
  {
    op(i);
  }
}

template<class Cn, class Op>
void for_each(Cn const& cn, Op const& op)
{
  if (cn.empty()) return;
  for (std::size_t i = 0; i < cn.size(); ++i)
  {
    op(cn[i]);
  }
}

template<class Cn, class Op1, class Op2>
void for_each(Cn const& cn, Op1 const& op_n, Op2 const& op_l)
{
  if (cn.empty()) return;
  for (std::size_t i = 0; i < cn.size() - 1; ++i)
  {
    op_n(cn[i]);
  }
  op_l(cn.back());
}

template<class Op1, class Op2>
void for_each(std::size_t const& n, Op1 const& op_n, Op2 const& op_l)
{
  if (n == 0) return;
  for (std::size_t i = 0; i < n - 1; ++i)
  {
    op_n(i);
  }
  op_l(n - 1);
}

} // namespace OB::Algorithm

#endif // OB_ALGORITHM_HH
