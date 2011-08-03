import re, csv

#
#
# Annex E Command Dictionary ---------------------------------------------
#
#


chunku = []

commands = r'''0001H C-STORE-RQ
8001H C-STORE-RSP
0010H C-GET-RQ
8010H C-GET-RSP
0020H C-FIND-RQ
8020H C-FIND-RSP
0021H C-MOVE-RQ
8021H C-MOVE-RSP
0030H C-ECHO-RQ
8030H C-ECHO-RSP
0100H N-EVENT-REPORT-RQ
8100H N-EVENT-REPORT-RSP
0110H N-GET-RQ
8110H N-GET-RSP
0120H N-SET-RQ
8120H N-SET-RSP
0130H N-ACTION-RQ
8130H N-ACTION-RSP
0140H N-CREATE-RQ
8140H N-CREATE-RSP
0150H N-DELETE-RQ
8150H N-DELETE-RSP
0FFFH C-CANCEL-RQ'''

chunku.append('''/* -----------------------------------------------------------------------
 *
 * Converted from Part 7: Message Exchange (2009)
 * Annex E Command Dictionary, E.1 REGISTRY OF DICOM COMMAND ELEMENTS
 *
 */

typedef enum {''')

for line in commands.splitlines():
    cmdval, cmdname = line.split()
    cmdname = cmdname.replace('-', '_')
    cmdval = '0x'+cmdval[:4]
    chunku.append('\t%s \t= %s,'%(cmdname, cmdval))
    
chunku.append('} commandtype;\n\n')

#
#
# uid registry -----------------------------------------------------------
#
#

# build uid list -------------------------------

uidlist = []
for r in [r[:4] for r in csv.reader(open('uid.csv'))]:
    uid_value, uid_name, uid_type, uid_part = r
    uid_value = uid_value.strip()
    uid_name = uid_name.strip()
    uid_type = uid_type.strip()
    uid_part = uid_part.strip()
    
    uid = uid_name.upper().replace(' & ', 'AND')
    uid = re.sub('['+re.escape(',-@/()')+']', ' ', uid)
    uid = uid.split(':')[0]
    uid = uid.split('[')[0]
    uid = '_'.join(uid.split())
    uid = 'UID_'+uid
    if 'RETIRED' in uid: uid = None
    
    uid_type = uid_type.strip()        
    if uid_type == 'Transfer': uid_type = 'Transfer Syntax'
    if uid_type not in ['Application Context Name',
                        'Coding Scheme',
                        'DICOM UIDs as a Coding Scheme',
                        'Meta SOP Class',
                        'Query/Retrieve',
                        'SOP Class',
                        'Service Class',
                        'Transfer Syntax',
                        '']:
        uid = None
    uidlist.append([uid_value, uid, uid_name, uid_type, uid_part])
uidlist.append(['', 'UID_UNKNOWN', '', '', ''])

for u in uidlist:
    if u[3] == '':   u[3] = '00'+u[3]
    if u[3] == 'Transfer Syntax':   u[3] = '01'+u[3]
    if u[3] == 'SOP Class':         u[3] = '02'+u[3]
    if u[3] == 'Meta SOP Class':    u[3] = '03'+u[3]
    if u[3] == 'Query/Retrieve':    u[3] = '04'+u[3]
    
uidlist.sort(key=lambda x: x[3])

# write to 'dicomdict.hxx' -------------------------------

chunku.append('''/* -----------------------------------------------------------------------
 *
 * Converted from DICOM Part 6: Data Dictionary (2009)
 *
 * Python codelet that converts uid name into 'uid variable name'.
 *
    uid = uid_name.upper().replace(' & ', 'AND')
    uid = re.sub('['+re.escape(',-@/()')+']', ' ', uid)
    uid = uid.split(':')[0]
    uid = uid.split('[')[0]
    uid = '_'.join(uid.split())
    uid = 'UID_'+uid
    if 'RETIRED' in uid: uid = None
 */

typedef enum {
\tUID_UNKNOWN = 0,''')

uid_a = []
for u in uidlist:
    if u[1]:
        uid_a.append(u[1])
        if u[1] != 'UID_UNKNOWN':
            chunku.append('\t%s,'%(u[1]))
chunku[-1] = chunku[-1][:-1]    
chunku.append('} uidtype;')

# dump!!
file('dicomdict.hxx', 'wb').write('\n'.join(chunku))

# -------------------------------

chunku = []

chunku.append('''static const struct _uid_registry_struct_ uid_registry[] =
{''')

uid_b = []
uidlist.sort()
for u in uidlist:
    if u[3].startswith('0'): u[3] = u[3][2:]
    chunku.append('\t{\t"%s",\n\t\t%s,'%(u[0], u[1] if u[1] else 'UID_UNKNOWN'))
    chunku.append('\t\t"%s", "%s", "%s"},'%(u[2], u[3], u[4]))
    uid_b.append(u[1])
chunku[-1] = chunku[-1][:-1]    

chunku.append('''};

static const char *uidvalue_registry[] = {''')

for u in uid_a:
    chunku.append('\tuid_registry[{0}].uidvalue,'
                  '\tuid_registry[{0}].uid_name,'.format(uid_b.index(u)))
chunku[-1] = chunku[-1][:-1]
chunku.append('''};''')

#
#    
# data element registry --------------------------------------------------
#
#

chunke = []
chunke.append(r'''/* -----------------------------------------------------------------------
 *
 * Converted from DICOM Part 6: Data Dictionary (2009)
 *            and DICOM Part 7: Message Exchange (2009)
 *
 */

static const struct _element_registry_struct_ element_registry[] =
{''')

keyword2tag = []
for r in [r[:6] for r in csv.reader(open('elements.csv'))]:
    tagstr, name, keyword, vrstr, vm, retire = r
    tagstr = tagstr.strip()
    name = name.strip()
    keyword = keyword.strip()
    vrstr = vrstr.strip()
    vm = vm.strip()
    retire = retire.strip()
    
    tag = tagstr.replace('x', '0').strip()
    tag = '0x'+''.join(tag[1:-1].split(','))
    
    if vrstr:
        vr = vrstr.split()[0]
    else:
        vr = 'NULL'
    
    if keyword:
        keyword2tag.append([keyword, tag])
    
    chunke.append(
        '\t{%s, "%s", "%s", "%s", VR_%s, "%s", "%s", %d},'%
        (tag, tagstr, name, keyword, vr, vrstr, vm, retire=="RET") )    
chunke[-1] = chunke[-1][:-1]

chunke.append('''};

static const struct _element_keyword_struct_ element_keyword[] =
{''')

keyword2tag.sort()
for r in keyword2tag:
    chunke.append('\t{%s, "%s"},'%(r[1], r[0]))
chunke[-1] = chunke[-1][:-1]    

chunke.append('};');

# dump!!
file('dicomdict.inc.cxx', 'wb').write('\n'.join(chunke+chunku))



