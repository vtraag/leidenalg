import unittest


class BaseTestCases:  

  class BaseTest(unittest.TestCase):
  
      def testCommon(self):
          print 'Calling BaseTest:testCommon'
          value = 5
          self.assertEquals(value, 5)

class SubTest1(BaseTestCases.BaseTest):

    def testSub1(self):
        print 'Calling SubTest1:testSub1'
        sub = 3
        self.assertEquals(sub, 3)


class SubTest2(BaseTestCases.BaseTest):

    def testSub2(self):
        print 'Calling SubTest2:testSub2'
        sub = 4
        self.assertEquals(sub, 4)

    def testCommon(self):
        print 'Calling SubTest2:testCommon'
        value = 5
        self.assertEquals(value, 5)

if __name__ == '__main__':
    unittest.main()