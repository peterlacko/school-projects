<?php
namespace Poradna;
use Nette;

/**
 *  * Tabulka user
 *   */
class FakultaRepository extends Repository
{
    public function showFacultiesNames()
    {
		return $this->findAll();
    }

    public function oborOf(Nette\Database\Table\ActiveRow $detailFakulty) {
		return $detailFakulty->related('obor')->order('Nazev_oboru');
    }

    public function dodOf(Nette\Database\Table\ActiveRow $detailFakulty) {
		return $detailFakulty->related('dod')->order('Zacatek');
    }
    
    public function predmetOf(Nette\Database\Table\ActiveRow $detailFakulty) {
		return $detailFakulty->related('predmet');
    }

    public function findFakultaInSkola($fakulta, $skola)
    {
	return $this->getTable()->where('Nazev_fakulty', $fakulta)->where('Nazev_skoly', $skola)->fetch();
    }

    public function addFakulta($values)
    {
	$this->getTable()->insert(array(
	    'Nazev_fakulty'=>$values->nazev_fakulty,
	    'Adresa_fakulty'=>$values->adresa_fakulty,
	    'Email_fakulty'=>$values->email_fakulty,
	    'Web_fakulty'=>$values->web_fakulty,
	    'Dekan'=>$values->dekan,
	    'Nazev_skoly'=>$values->skola
	));
	if ($values->logo_fakulty != '') {
	    $logo = $values->logo_fakulty->toImage();
	    $logo->save(__DIR__.'/../../www/images/logos/'.$this->getTable()->max('ID_fakulty').'.jpg', 98);
	}
    }


    public function deleteFakulta($id)
    {
	$this->findBy(array('ID_fakulty'=>$id))->delete();
    }

    public function search($phrase)
    {
		return $this->connection->table('fakulta')
			->where("Nazev_fakulty LIKE ? OR Adresa_fakulty LIKE ? OR Dekan LIKE ?", "%$phrase%", "%$phrase%", "%$phrase%");
    }
}
